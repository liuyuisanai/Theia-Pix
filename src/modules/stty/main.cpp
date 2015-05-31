extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <nuttx/config.h>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

inline bool
streq(const char *a, const char *b) { return not std::strcmp(a, b); }

bool
parse_uint(const char s[], uint32_t &n, const char * & tail)
{
	char *p;
	n = std::strtoul(s, &p, 10);
	tail = p;
	return tail != s;
}

bool
parse_uint(const char s[], uint32_t &n)
{
	const char * tail = nullptr;
	return parse_uint(s, n, tail) and tail and *tail == '\0';
}

bool
parse_baudrate(const char s[], termios & tty)
{
	/* NuttX baud rate constants Bn are equal to n.  *
	 * Warning: They are not equal on all palrforms. */
	uint32_t baudrate;
	bool ok = parse_uint(s, baudrate);
	if (ok)
	{
		ok = cfsetspeed(&tty, baudrate) == 0;
		if (not ok) { perror("cfsetspeed"); }
	}
	return ok;
}

bool
parse_bits_parity_stopbits(const char s[], termios & tty)
{
	bool ok = strlen(s) == 3;
	if (not ok) { return false; }

	int bits_per_frame, stopbits, parity;
	switch (s[0])
	{
#ifdef CS5
	case '5':
		 bits_per_frame = CS5;
		 break;
#endif
#ifdef CS6
	case '6':
		 bits_per_frame = CS6;
		 break;
#endif
#ifdef CS7
	case '7':
		 bits_per_frame = CS7;
		 break;
#endif
#ifdef CS8
	case '8':
		 bits_per_frame = CS8;
		 break;
#endif
	default:
		 return false;
	}

	switch (s[1])
	{
	case 'N': case 'n':
		 parity = 0;
		 break;
	case 'E': case 'e':
		 parity = PARENB;
		 break;
	case 'O': case 'o':
		 parity = PARODD;
		 break;
	default:
		 return false;
	}
	switch (s[2])
	{
	case '1':
		 stopbits = 0;
		 break;
	case '2':
		 stopbits = CSTOPB;
		 break;
	default:
		 return false;
	}

	tty.c_cflag &= ~CSIZE | ~CSTOPB | ~PARENB | ~PARODD;
	tty.c_cflag |= bits_per_frame | parity | stopbits;

	return true;
}

bool
parse_ctsrts(const char s[], termios & tty)
{
	int or_mask = 0, nand_mask = 0;

	if (streq(s, "+cts"))         { or_mask = CCTS_OFLOW; }
	else if (streq(s, "-cts"))    { nand_mask = CCTS_OFLOW; }
	else if (streq(s, "+ctsrts")) { or_mask = CCTS_OFLOW | CRTS_IFLOW; }
	else if (streq(s, "-ctsrts")) { nand_mask = CCTS_OFLOW | CRTS_IFLOW; }
	else if (streq(s, "+rts"))    { or_mask = CRTS_IFLOW; }
	else if (streq(s, "-rts"))    { nand_mask = CRTS_IFLOW; }
	else { return false; }

	tty.c_cflag &= ~nand_mask;
	tty.c_cflag |= or_mask;

	return true;
}

bool
apply(int fd, int argc, const char * const argv[])
{
	termios tty;

	bool ok = tcgetattr(fd, &tty) == 0;
	if (not ok)
	{
		perror("tcgetattr");
		return false;
	}

	while (argc > 0)
	{
		const char * const s = *argv;
		ok = parse_baudrate(s, tty)
			or parse_bits_parity_stopbits(s, tty)
			or parse_ctsrts(s, tty);

		if (not ok) { fprintf(stderr, "Invalid parameter: %s.\n", s); }

		--argc;
		++argv;
	}

	ok = tcsetattr(fd, TCSANOW, &tty) == 0;
	if (not ok) { perror("tcsetattr"); }

	return ok;
}

void
show_baudrate(const termios & tty)
{ printf("Baudrate: %i %i.\n", cfgetispeed(&tty), cfgetospeed(&tty)); }

void
show_bits_parity_stopbits(termios & tty)
{
	char cs, par, stops;
	switch (tty.c_cflag & CSIZE)
	{
#ifdef CS5
	case CS5:
		cs = '5';
		break;
#endif
#ifdef CS6
	case CS6:
		cs = '6';
		break;
#endif
#ifdef CS7
	case CS7:
		cs = '7';
		break;
#endif
#ifdef CS8
	case CS8:
		cs = '8';
		break;
#endif
	default:
		cs = '?';
		fprintf(stderr, "Unknown CS.\n");
		break;
	}
	par = tty.c_cflag & PARENB ? (tty.c_cflag & PARODD ? 'o' : 'e') : 'n';
	stops = tty.c_cflag & CSTOPB ? '2' : '1';
	printf("%c%c%c\n", cs, par, stops);
}

void
show_ctsrts(const termios & tty)
{
	printf("%ccts %crts\n"
			, tty.c_cflag & CCTS_OFLOW ? '+' : '-'
			, tty.c_cflag & CRTS_IFLOW ? '+' : '-'
	);
}

void
show(int fd)
{
	termios tty;

	bool ok = tcgetattr(fd, &tty) == 0;
	if (not ok)
	{
		perror("tcgetattr");
		return;
	}

	show_baudrate(tty);
	show_bits_parity_stopbits(tty);
	show_ctsrts(tty);
	printf("\n");
}

void
usage(const char name[])
{
	fprintf(stderr, "Usage: %s tty [parameters]\n"
			"Parameters:\n"
			"  baudrate\n"
			"  {5,6,7,8}{n,e,o}{1,2}\n"
			"  {+,-}{cts,ctsrts,rts}\n"
			"\n", name);
}

} // end of namespace

int
main(int argc, const char * const * const argv)
{
	if (argc < 2)
	{
		usage(argv[0]);
		return 1;
	}

	int fd = open(argv[1], 0);

	if (fd < 0) { perror("open"); }
	else
	{
		if (argc == 2) { show(fd); }
		else
		{
			bool ok = apply(fd, argc - 2, argv + 2);
			if (not ok) { usage(argv[0]); }
		}
		close(fd);
	}

	return 0;
}
