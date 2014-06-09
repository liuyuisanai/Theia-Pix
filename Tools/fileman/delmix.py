#!/usr/bin/env python
#
# /fs/microsd/etc/mixers/FMU_quad_x.mix file deleter
#

from nsh import NSH

def main():
	fn = "/fs/microsd/etc/mixers/FMU_quad_x.mix"

	try:
		nsh = NSH()
		nsh.wait_and_open_nsh()
		if nsh.file_exists(fn):
			nsh.exec_cmd("rm {}".format(fn), 2.0)
			print("Success.")
		else:
			print("Cannot find {}".format(fn))

	finally:
		nsh.close()

if __name__ == "__main__":
	main()
