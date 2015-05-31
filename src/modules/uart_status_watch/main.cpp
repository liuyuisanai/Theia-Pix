extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <nuttx/config.h>
#include <stm32_uart.h>

#include <fcntl.h>
#include <unistd.h>

#include <cstdio>

#ifndef CONFIG_ARCH_CHIP_STM32
# error Supported only STM32.
#endif

namespace {

inline bool
kbd_break()
{
	char c = 0;
	return read(0, &c, 1) == 1 and c == '\3';
}

static const struct { uint8_t n; uint32_t const * addr; }
STATUS_ADDRESS[] =
{
#ifdef CONFIG_STM32_USART1
	{ 1, (const uint32_t*)(STM32_USART1_BASE + STM32_USART_SR_OFFSET) },
#endif
#ifdef CONFIG_STM32_USART2
	{ 2, (const uint32_t*)(STM32_USART2_BASE + STM32_USART_SR_OFFSET) },
#endif
#ifdef CONFIG_STM32_USART3
	{ 3, (const uint32_t*)(STM32_USART3_BASE + STM32_USART_SR_OFFSET) },
#endif
#ifdef CONFIG_STM32_USART4
	{ 4, (const uint32_t*)(STM32_USART4_BASE + STM32_USART_SR_OFFSET) },
#endif
#ifdef CONFIG_STM32_USART5
	{ 5, (const uint32_t*)(STM32_USART5_BASE + STM32_USART_SR_OFFSET) },
#endif
#ifdef CONFIG_STM32_USART6
	{ 6, (const uint32_t*)(STM32_USART6_BASE + STM32_USART_SR_OFFSET) },
#endif
#ifdef CONFIG_STM32_USART7
	{ 7, (const uint32_t*)(STM32_USART7_BASE + STM32_USART_SR_OFFSET) },
#endif
#ifdef CONFIG_STM32_USART8
	{ 8, (const uint32_t*)(STM32_USART8_BASE + STM32_USART_SR_OFFSET) },
#endif
};

constexpr size_t
N_STATUS_ADDRESS = sizeof STATUS_ADDRESS / sizeof *STATUS_ADDRESS;

static_assert(0 < N_STATUS_ADDRESS and N_STATUS_ADDRESS <= 8,
		"Bad UART Status Register count.");

void
dump_uart_status()
{
	printf("\tCTS\tLBD\tTXE\tTC\tRXNE\tIDLE\tORE\tNF\tFE\tPE\n");
	for (unsigned i = 0; i < N_STATUS_ADDRESS; ++i)
	{
		uint32_t st = *STATUS_ADDRESS[i].addr;
		printf("%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n"
			, STATUS_ADDRESS[i].n
			, (st & 0x200) != 0
			, (st & 0x100) != 0
			, (st & 0x080) != 0
			, (st & 0x040) != 0
			, (st & 0x020) != 0
			, (st & 0x010) != 0
			, (st & 0x008) != 0
			, (st & 0x004) != 0
			, (st & 0x002) != 0
			, (st & 0x001) != 0
		);
	}
}

} // end of namespace

int
main(int argc, const char * const * const argv)
{
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
	while (not kbd_break())
	{
		usleep(20000);
		printf("\f\n");
		dump_uart_status();
	}

	return 0;
}
