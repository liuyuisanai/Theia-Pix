#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include "binary_format.hpp"
#include "flash.hpp"
#include "unique_file.hpp"

extern "C" __EXPORT int
main(int argc, char * argv[])
{
	using namespace SiKUploader;

	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s tty firmware [-at]\n\n"
				"Firmware has to be in binary record format."
				" See firmware2bin.py.\n"
				"\n",
				argv[0]
		);
		return 1;
	}

	const char * const tty = argv[1];
	const char * const fname = argv[2];
	bool no_at = argc >=4 and !strcmp(argv[3], "-at");

	unique_file f = open(fname, O_RDONLY);
	if (f.get() == -1)
	{
		perror("open firmware");
		return 1;
	}

	if (not is_firmware_file_valid(f.get()))
	{
		fprintf(stderr, "Invalid firmware header.\n");
		return 1;
	}

	fprintf(stderr, "Firmware is valid.\n");

	unique_file d = open(tty, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (d.get() == -1)
	{
		perror("open tty");
		return 1;
	}

	fprintf(stderr, "TTY opened.\n");

	bool ok = no_at or AT_UPDATE(d.get());
	ok = ok and flash_from_file(f.get(), d.get());
	if (not ok)
	{
		fprintf(stderr, "Firmware upload failed.\n");
		return 1;
	}

	fprintf(stderr, "Firmware upload succeeded.\n");
	return 0;
}
