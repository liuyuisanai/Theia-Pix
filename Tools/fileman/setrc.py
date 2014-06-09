#!/usr/bin/env python
#
# /fs/microsd/etc/rc.txt file uploader
#

from nsh import NSH

from os import path
from sys import argv, exit

def main():
	if len(argv) != 2:
		print("usage: setrc <path to rc.txt>")
		exit(1)

	rc_path = argv[1]
	if not path.isfile(rc_path):
		print("cannot find file {}".format(rc_path))
		exit(2)

	try:
		nsh = NSH()
		nsh.wait_and_open_nsh()
		if nsh.upload_file("/fs/microsd/etc/rc.txt", rc_path):
			print("Success.")
	finally:
		nsh.close()

if __name__ == "__main__":
	main()
