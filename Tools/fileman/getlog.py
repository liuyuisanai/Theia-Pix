#!/usr/bin/env python
#
# Log file downloader
#

from nsh import NSH
from sys import argv, exit
from datetime import datetime
import re

def main():
	if(len(argv) != 2):
		print("usage: getlog [last|list|log_path]")
		exit(1)

	cmd = argv[1]
	logs_dir = "/fs/microsd/log"

	try:
		nsh = NSH()
		nsh.wait_and_open_nsh()
		if cmd == "list":
			for file in nsh.get_all_files(logs_dir):
				print(file)
		elif cmd == "all":
			for file in nsh.get_all_files(logs_dir):
				with open(fn[fn.rindex('/')+1:], "wb") as f:
					data = nsh.download_file(logs_dir + file)
					print("Writing buffer to file..")
					f.write(data)
					print("Success.")
		elif cmd == "last":
			dirs = nsh.ls_dir(logs_dir)
			# Keep entries only "date/" entries
			dirs = filter(lambda d: re.sub("[0-9]{4}\-[0-9]{2}\-[0-9]{2}", '', d) == "/", dirs)
			if dirs:
				dirs.sort(key=lambda x: datetime.strptime(x, '%Y-%m-%d/'))

				last_dir = dirs.pop()
				print("Getting logs from {}/{}".format(logs_dir, last_dir))

				logs = nsh.ls_dir(logs_dir + '/' + last_dir)
				# Keep entries only which end with .bin
				logs = filter(lambda l: l.endswith('.bin'), logs)
				if logs:
					logs.sort(key=lambda x: datetime.strptime(x, "%H_%M_%S.bin"))

					last_log = logs.pop()
					data = nsh.download_file(logs_dir + '/' + last_dir + last_log)
					with open(last_log, "wb") as f:
						f.write(data)

					preflight = "preflight_perf" + last_log[:-3] + "txt"
					if nsh.file_exists(logs_dir + '/' + last_dir + preflight):
						data = nsh.download_file(logs_dir + '/' + last_dir + preflight)
						with open(preflight, "wb") as f:
							f.write(data)
					else:
						print("Cannot find {} file (ignoring it).".format(preflight))

					print("Success.")
				else:
					print("Cannot find any log with timestamp!")
			else:
				print("Unable to find any log with date")
		else:
			fn = logs_dir + "/" + cmd
			if nsh.file_exists(fn):
				data = nsh.download_file(fn)
				with open(fn[fn.rindex('/')+1:], "wb") as f:
					f.write(data)
				print("Success.")
			else:
				print("File {} does not exists!")
	finally:
		nsh.close()

if __name__ == "__main__":
	main()

