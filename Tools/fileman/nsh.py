import os, sys, serial, time, struct

class NSH:
	def __init__(self):
		self.serial = None

	def flush(self):
		self.serial.flush()

	def send(self, data):
		from collections import deque
		data = deque(data)
		attempt = 0
		while data:
			if self.serial.write(data.popleft()) == 0:
				print("Failed to write {}".format(attempt))
				attempt += 1
				if attempt > 50:
					raise RuntimeError("fail")
			else:
				attempt = 0
			self.serial.flush()

	def recv(self, size=1):
		if size is not None:
			attempt = 1
			data = self.serial.read(size)
			if len(data) < 1:
				raise RuntimeError("timeout waiting for data")
			size -= len(data)
			if size > 0:
				#raise RuntimeError("received less data than expected ({}/{})".format(len(data), size))
				attempt += 1
				if attempt > 10:
					raise RuntimeError("Couldn't receive all the data ({} bytes left)".format(size))
			return data
		else:
			return self.serial.read()

	def wait_for_string(self, s, timeout, debug=False):
		t0 = time.time()
		buf = []
		res = []
		n = 0
		if type(s) == str:
			s = (s,)
		maxlen = max((len(string) for string in s))
		while True:
			c = self.recv(size=None)
			if debug:
				sys.stderr.write(c)
			buf.append(c)
			if len(buf) > maxlen:
				res.append(buf.pop(0))
				n += 1
				if n % 10000 == 0:
					sys.stderr.write(str(n) + "\n")
			string = "".join(buf)
			for sz in s:
				index = string.find(sz)
				if index != -1:
					res += buf[:index]
					return "".join(res)
			if timeout > 0.0 and time.time() - t0 > timeout:
				raise Exception("Timeout while waiting for: {}".format(s))

	def exec_cmd(self, cmd, timeout, cmd_end_str=("nsh> \x1b[K",)):
		print("nsh> {}".format(cmd))
		self.send(cmd + "\n")
		self.flush()
		self.wait_for_string(cmd + "\r\n", timeout)
		if cmd_end_str:
			return self.wait_for_string(cmd_end_str, timeout)
		else:
			return None

	def identify(self):
		# ver hw
		#PX4FMU_V2 or Airdog or AirLeash
		_hw_version = self.exec_cmd("ver hw", 1.0)
		print(_hw_version)
		#return (_hw_version.find("PX4FMU_V2") >= 0 or _hw_version.find("AIRDOG_FMU") >= 0 or _hw_version.find("AirLeash") >= 0)
		return True #Skip board version check


	def ls_dir(self, dir, timeout=1.0):
		res = []
		dir = dir.rstrip("/")
		for line in self.exec_cmd("ls " + dir, timeout).splitlines()[1:]:
			res.append(line[1:])
		return res

	def get_dir_files(self, directory="/"):
		res = []
		for f in self.ls_dir(directory):
			if f[-1] != '/': #is file
				res.append(f)
		return f

	def get_all_files(self, basedir="/", subdir="/"):
		dir = basedir + subdir
		res = []
		for d in self.ls_dir(dir):
			if d[-1] == '/':
				res += self.get_all_files(basedir, subdir + d)
			else:
				res.append(subdir + d)
		return res

	def get_all_dirs(self, basedir="/", subdir="/"):
		dir = basedir + subdir
		res = []
		for d in self.ls_dir(dir):
			if d[-1] == '/':
				res += self.get_all_files(basedir, subdir + d)
				res.append(d)
		return res

	def file_exists(self, fn):
		rslash = fn.rindex("/")
		dir = fn[:rslash]
		fn = fn[rslash+1:]
		found = False
		for file in self.ls_dir(dir):
			if file == fn:
				return True
		return False

	def download_file(self, fn):
		res = self.exec_cmd("dumpfile " + fn, 1.0, ("\n",))
		data = []
		if res.startswith("OK"):
			size = int(res.split()[1])
			block_size = 1024 * 16
			print("Reading file {}:".format(fn))
			while size > 0:
				buf = self.recv(min(size, block_size))
				data.append(buf)
				size -= len(buf)
				print("Read {} bytes / {} left".format(len(buf), size))
				sys.stdout.flush()
			print("Successfully read file {}".format(fn))
		else:
			raise Exception("Error downloading file (not exists?)")
		self.wait_for_string("nsh> \x1b[K", 2.0)
		return "".join(data)

	def random_str(self, length):
		from random import shuffle
		from math import ceil
		abc = ''.join([chr(c) for c in range(ord('a'), ord('z') + 1)])
		ABC = ''.join([chr(c) for c in range(ord('A'), ord('Z') + 1)])
		num = '0123456789'
		string = abc + ABC + num
		mul = int(ceil(float(length) / len(string)))
		res = list(string * mul)
		shuffle(res)
		return ''.join(res[:length])

	def upload_file(self, out_fn, in_fn):
		f = open(in_fn, 'rb')
		filedata = f.read()
		f.close()

		szOK = "OK\r\n"
		szDEFEND = "nsh> \x1b[K"

		while True:
			tmp_fn = '/fs/microsd/' + self.random_str(14) + '.tmp'
			if not self.file_exists(tmp_fn):
				break

		res = self.exec_cmd("writefile {} {}".format(tmp_fn, len(filedata)), 1.0, (szDEFEND, szOK))
		if len(res) == 0: #szOK found
			self.flush()
			print("Writing file {}".format(tmp_fn))
			self.send(filedata)
			self.wait_for_string("nsh> \x1b[K", 2.0)
			print("Deleting {}".format(out_fn))
			self.exec_cmd("rm {}".format(out_fn), 2.0)
			print("Coping {} to {}".format(tmp_fn, out_fn))
			self.exec_cmd("cp {} {}".format(tmp_fn, out_fn), 2.0)
			print("Removing temponary file {}".format(tmp_fn))
			self.exec_cmd("rm {}".format(tmp_fn), 2.0)
			return True
		else: #nsh> found
			print("Error occured during the file upload (`{}` < `{}`)".format(out_fn, in_fn))
			print(res)
			return False

	def wait_and_open_nsh(self):
		print("Waiting for the device.. (Press Ctrl+C to stop)")
		while True:
			if os.name == 'nt':
				port_list = ['COM32', 'COM31', 'COM30', 'COM29', 'COM28', 'COM27', 'COM26', 'COM25', 'COM24', 'COM23', 'COM22', 'COM21', 'COM20', 'COM19', 'COM18', 'COM17', 'COM16', 'COM15', 'COM14', 'COM13', 'COM12', 'COM11', 'COM10', 'COM9', 'COM8', 'COM7', 'COM6', 'COM5', 'COM4', 'COM3', 'COM2', 'COM1', 'COM0']
			else:
				from glob import glob
				port_list = glob("/dev/serial/by-id/usb-3D_Robotics*") #linux
				port_list += glob("/dev/tty.usbmodem*") #mac

			for port in port_list:
				try:
					self.serial = serial.Serial(port, 57600, timeout=0.5)
				except serial.SerialException:
					#open failed, rate-limit our attempts
					time.sleep(0.05)
					# and loop to the next port
					continue

				# port is open, try talking to it
				# identify the bootloader
				if self.identify():
					print("Found device on port {}".format(port))
					return
				else:
					print("Ignoring invalid board on port {}".format(port))
					self.close()
					continue

	def close(self):
		if self.serial is not None:
			self.serial.close()

if __name__ == "__main__":
	print("This is class file. It does nothing.")

