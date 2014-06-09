#!/usr/bin/env python
#
# /fs/microsd/etc/mixers/FMU_quad_x.mix file downloader
#

from nsh import NSH

def main():
	fn = "/fs/microsd/etc/mixers/FMU_quad_x.mix"

	if len(argv) == 1:
		out_fn = fn[fn.rindex('/')+1:]
	else:
		out_fn = argv[1]


	try:
		nsh = NSH()
		nsh.wait_and_open_nsh()
		if nsh.file_exists(fn):
			with open(out_fn, "wb") as f:
				data = nsh.download_file(fn)
				print("Writing buffer to file..")
				f.write(data)
				print("Success.")
		else:
			print("Cannot find {}".format(fn))

	finally:
		nsh.close()

if __name__ == "__main__":
	main()

