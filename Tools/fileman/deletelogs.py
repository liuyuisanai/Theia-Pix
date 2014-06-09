#!/usr/bin/env python
#
# Logs deleter
#

from nsh import NSH

def main():
	try:
		logs_dir = "/fs/microsd/log"
		nsh = NSH()
		nsh.wait_and_open_nsh()
		for file in nsh.get_all_files(logs_dir):
			print("Removing file: {}{}".format(logs_dir, file))
			nsh.exec_cmd("rm {}{}".format(logs_dir, file), 2.0)
		for dir in nsh.get_all_dirs(logs_dir):
			print("Removing dir: {}/{}".format(logs_dir, dir))
			nsh.exec_cmd("rmdir {}/{}".format(logs_dir, dir[:-1]), 1.0)
		print("Done.")
	finally:
		nsh.close()

if __name__ == "__main__":
	main()
