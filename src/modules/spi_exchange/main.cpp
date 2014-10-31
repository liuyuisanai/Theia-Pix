extern "C" __EXPORT int main(int argc, const char * const * argv);

#include <nuttx/config.h>
#include <nuttx/spi.h>

#include <cstdlib>
#include <cstdio>

#define MAX_LEN 64

namespace {

struct spi_dev_s *
init(int bus_no)
{ return up_spiinitialize(bus_no); }

void
exchange(spi_dev_s * dev, int dev_no, uint8_t send[], uint8_t recv[], size_t len)
{
	SPI_LOCK(dev, true);

	//SPI_SETFREQUENCY(_dev, _frequency);
	//SPI_SETMODE(_dev, _mode);
	SPI_SETBITS(dev, 8);
	SPI_SELECT(dev, (spi_dev_e)dev_no, true);

	SPI_EXCHANGE(dev, send, recv, len);

	/* and clean up */
	SPI_SELECT(dev, (spi_dev_e)dev_no, false);
	SPI_LOCK(dev, false);
}

bool
parse_uint(const char s[], unsigned &n)
{
	char *p;
	n = std::strtoul(s, &p, 0);
	return *p == '\0';
}

uint8_t
decode_hexdigit(uint8_t x)
{
	if (x < '0')
		return 0;
	if (x <= '9')
		return x - (uint8_t)'0';
	if (x < 'A')
		return 0;
	if (x <= 'F')
		return x - (uint8_t)'A' + 0xAu;
	if (x < 'a')
		return 0;
	if (x <= 'f')
		return x - (uint8_t)'a' + 0xAu;
	return 0;
}

uint8_t
decode_hex_8bit(char h, char l)
{ return (decode_hexdigit(h) << 4) | decode_hexdigit(l); }

size_t
decode_hex(const char *s, uint8_t buf[], size_t max_len)
{
	size_t i = 0;
	while (i < max_len and *s)
	{
		char l, h;
		h = *s;
		++s;
		l = *s;
		++s;
		buf[i] = decode_hex_8bit(h, l);
		++i;
	}
	return i;
}

void print_hex_8bit(uint8_t x)
{ printf("%02x", x); }

void
print_hex(const uint8_t * buf, size_t len)
{
	while (len) {
		print_hex_8bit(*buf);
		++buf;
		--len;
	}
}

} // end of namespace

int
main(int argc, const char * const * argv)
{
	int i = 0;
	unsigned bus_no;
	unsigned dev_no;
	struct spi_dev_s * dev;
	uint8_t buf[MAX_LEN];

	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s BUS DEV hex-string ...\n"
				"Like: %s 1 1 f0a3b5f0a3b5 F0A3B5F0A3B5\n",
				argv[0], argv[0]);
		return 1;
	}
	++i;
	++argv;

	if (not parse_uint(*argv, bus_no))
	{
		fprintf(stderr, "Invalid bus number number: %s\n", *argv);
		return 1;
	}
	++i;
	++argv;

	if (not parse_uint(*argv, dev_no))
	{
		fprintf(stderr, "Invalid device number number: %s\n", *argv);
		return 1;
	}
	++i;
	++argv;

	dev = init(bus_no);
	if (dev == nullptr)
	{
		fprintf(stderr, "Failed to initialize SPI BUS %u\n", bus_no);
		return 1;
	}

	for(; i < argc; ++i, ++argv)
	{
		size_t len = decode_hex(*argv, buf, sizeof buf);

		printf("sent ");
		print_hex(buf, len);
		printf("\n");

		exchange(dev, dev_no, buf, buf, len);

		printf("recv ");
		print_hex(buf, len);
		printf("\n");
	}

	return 0;
}
