extern "C" __EXPORT int main(int argc, const char * const * argv);

#include <nuttx/config.h>
#include <nuttx/spi.h>

#include <cstring>
#include <cstdio>

#include <board_config.h>

namespace {

namespace spi {

constexpr uint8_t READ(uint8_t r) { return r | 0x80; }
constexpr uint8_t AUTOINC(uint8_t r) { return r | 0x40; }

struct spi_dev_s *
init(int bus) {
	auto dev = up_spiinitialize(bus);
	SPI_SETFREQUENCY(dev, 1000000);
	//SPI_SETMODE(dev, _mode);
	//SPI_SETBITS(dev, 8);
	return dev;
}

void
exchange(spi_dev_s * dev, int dev_no, uint8_t send[], uint8_t recv[], size_t len)
{
	SPI_LOCK(dev, true);
	SPI_SELECT(dev, (spi_dev_e)dev_no, true);

	SPI_EXCHANGE(dev, send, recv, len);

	SPI_SELECT(dev, (spi_dev_e)dev_no, false);
	SPI_LOCK(dev, false);
}

inline uint8_t
read_reg(spi_dev_s * dev, int dev_no, uint8_t reg)
{
	uint8_t buf[2] = { READ(reg), 0 };
	exchange(dev, dev_no, buf, buf, sizeof(buf));
	return buf[1];
}

bool
probe_gyro_L3GD20(spi_dev_s * dev)
{ return read_reg(dev, PX4_SPIDEV_GYRO, 0x0f) == 0xd4; }

bool
probe_accel_mag_LSM303D(spi_dev_s * dev)
{ return read_reg(dev, PX4_SPIDEV_ACCEL_MAG, 0x0f) == 0x49; }

bool
probe_gyro_accel_MPU6000(spi_dev_s * dev)
{
	uint8_t revisions[] = { 0x14, 0x15, 0x16, 0x17, 0x18, 0x54, 0x55, 0x56,
				0x57, 0x58, 0x59, 0x5A };
	uint8_t id = read_reg(dev, PX4_SPIDEV_MPU, 0x0c);
	return memchr(revisions, id, sizeof(revisions)) != nullptr;
}

bool
probe_baro_MS5611(spi_dev_s * dev)
{
	uint8_t buf[3] = {0, 0, 0};

	buf[0] = 0x1e;
	exchange(dev, PX4_SPIDEV_BARO, buf, buf, 1);

	bool ok;
	uint8_t i = 0;
	do {
		buf[0] = 0xa0 | i;
		buf[1] = 0;
		buf[2] = 0;

		exchange(dev, PX4_SPIDEV_BARO, buf, buf, 3);

		ok = (buf[1] != buf[2]) or ((buf[1] != 0) and (buf[1] != 0xff));

		i += 2;
	} while(not ok and i < 16);

	return ok;
}

#ifdef SPI_HMC5883_BUS
uint8_t
probe_mag_HMC5883(spi_dev_s * dev)
{
	uint8_t buf[4] = { READ(AUTOINC(0x0a)), 0, 0, 0 };
	exchange(dev, SPI_HMC5883_DEV, buf, buf, sizeof(buf));
	return buf[1] == 'H' and buf[2] == '4' and buf[3] == '3';
}
#endif

inline void
print_status(const char sens[], bool ok)
{ printf("spi %s: %s\n", sens, ok ? "ok" : "fail"); }

bool
report()
{
	struct spi_dev_s * dev = init(PX4_SPI_BUS_SENSORS);
	bool r = dev != nullptr;
	bool ok;

	if (r)
	{
		ok = probe_gyro_L3GD20(dev);
		r = r and ok;
		print_status("Gyro L3GD20", ok);

		ok = probe_accel_mag_LSM303D(dev);
		r = r and ok;
		print_status("Accel+Mag LSM303D", ok);

		ok = probe_gyro_accel_MPU6000(dev);
		r = r and ok;
		print_status("Gyro+Accel MPU6000", ok);

		ok = probe_baro_MS5611(dev);
		r = r and ok;
		print_status("Baro MS5611", ok);

#ifdef SPI_HMC5883_BUS
		ok = probe_mag_HMC5883(dev);
		r = r and ok;
		print_status("Mag HMC5883", ok);
#endif
	}
	else { printf("SPI sensor bus init failed.\n"); }

	return r;
}

} // end of namespace spi

} // end of namespace

int
main(int argc, const char * const * argv)
{ return spi::report() ? 0 : 1; }
