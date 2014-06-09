/**
 * @file writefile.c
 *
 * Write file utility. Writes file from stdin.
 *
 * @author Richard Dzenis <rixript@gmail.com>
 */

#include <nuttx/config.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>

#include <systemlib/err.h>

__EXPORT int writefile_main(int argc, char *argv[]);

static
ssize_t
min(ssize_t v1, ssize_t v2)
{
	return v1 > v2 ? v2 : v1;
}

int
writefile_main(int argc, char *argv[])
{
	if (argc < 3) {
		errx(1, "usage: writefile <filename> <filesize in bytes>");
	}

	/* open output file */
	FILE *f;
	f = fopen(argv[1], "w");

	if (f == NULL) {
		printf("ERROR opening file\n");
		exit(1);
	}
	
	ssize_t size = atol(argv[2]);
	if(size <= 0) {
		warnx("ERROR invalid size");
	}

	/* configure stdin */
	int in = fileno(stdin);

	struct termios tc;
	struct termios tc_old;
	tcgetattr(in, &tc);

	/* save old terminal attributes to restore it later on exit */
	memcpy(&tc_old, &tc, sizeof(tc));

	/* don't add CR on each LF*/
	tc.c_oflag &= ~ONLCR;

	if (tcsetattr(in, TCSANOW, &tc) < 0) {
		warnx("ERROR setting stdin attributes");
		exit(1);
	}

	char buf[512];
	ssize_t nread = 0;

	printf("OK\r\n");

	/* read stdin, and write to file */
	while ((nread = read(in, buf, min(sizeof(buf), size))) > 0) {
		if (fwrite(buf, 1, nread, f) != nread) {
			warnx("ERROR writing file");
			break;
		}
		size -= nread;
		if (size <= 0)
			break;
	}

	fclose(f);

	/* restore old terminal attributes */
	if (tcsetattr(in, TCSANOW, &tc_old) < 0) {
		warnx("ERROR restoring stdin attributes");
		exit(1);
	}

	return OK;
}
