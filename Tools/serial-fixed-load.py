import sys
import time

import serial

def main():
	portname, baud, freq, size, count = sys.argv[1:]
	p = serial.Serial(
		portname, int(baud), rtscts=False,
		#bytesize=8, parity='E', stopbits=2
	)
	print(p)

	data = bytes(0xFF & x for x in range(int(size)))

	count = int(count)
	interval = 1. / float(freq)
	n = 0
	stamp0 = start = time.time()
	while n < count:
		n += 1
		stamp1 = time.time()
		print("%4i  %9.3fs   pause %.4fs" %
			(n, stamp1 - start, stamp1 - stamp0))
		p.write(data)
		stamp0 = time.time()
		if stamp0 < stamp1 + interval:
			time.sleep(stamp1 + interval - stamp0)

if __name__ == "__main__":
        main()
