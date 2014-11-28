#include <stm32_gpio.h>
#include <stm32_fsmc.h>

#define FMC_BCR_CCLKEN (1 << 20)

#include "display.h"
#include "board_config.h"

void up_display_fmc()
{
	stm32_configgpio(GPIO_FSMC_D0);
	stm32_configgpio(GPIO_FSMC_D1);
	stm32_configgpio(GPIO_FSMC_D2);
	stm32_configgpio(GPIO_FSMC_D3);
	stm32_configgpio(GPIO_FSMC_D4);
	stm32_configgpio(GPIO_FSMC_D5);
	stm32_configgpio(GPIO_FSMC_D6);
	stm32_configgpio(GPIO_FSMC_D7);

	stm32_configgpio(GPIO_LCD_A0);
	stm32_configgpio(GPIO_LCD_RD);
	stm32_configgpio(GPIO_LCD_WR);
	stm32_configgpio(GPIO_LCD_CS);

	uint32_t reg;

	reg = getreg32(STM32_FSMC_BCR1);
	reg |= FSMC_BCR_WREN | FSMC_BCR_MBKEN;
	putreg32(reg, STM32_FSMC_BCR1);

	reg = getreg32(STM32_FSMC_BTR1);
	reg &= ~(FSMC_BTR_BUSTURN_MASK | FSMC_BTR_DATAST_MASK | FSMC_BTR_ADDSET_MASK);
	reg |= FSMC_BTR_BUSTRUN(15) | FSMC_BTR_DATAST(14) | FSMC_BTR_ADDSET(11);
	putreg32(reg, STM32_FSMC_BTR1);
}

void fmc_enable()
{
	uint32_t regval = getreg32( STM32_RCC_AHB3ENR);
	regval |= RCC_AHB3ENR_FSMCEN;
	putreg32(regval, STM32_RCC_AHB3ENR);
}

void fmc_disable()
{
	uint32_t regval = getreg32( STM32_RCC_AHB3ENR);
	regval &= ~RCC_AHB3ENR_FSMCEN;
	putreg32(regval, STM32_RCC_AHB3ENR);
}
