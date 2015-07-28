#include "pwm_led.h"

#include <stdio.h>

#include <board_config.h>
#include <stm32.h>
#include <stm32_gpio.h>
#include <stm32_tim.h>

#define TIMER_COUNT 12

static struct {
    int active;
    int intensity;
} leds[PWM_LED_SIZE] = {
{
    0, 100
},
{
    0, 100
}
};

#define REG(_reg)	(*(volatile uint32_t *)(_reg))
#define TIMER_CR1_EN (1 << 0)

#define GPIO_TIM1_CH3OUT_RED \
    (GPIO_ALT|GPIO_AF1|GPIO_SPEED_50MHz|GPIO_PUSHPULL|GPIO_PORTE|GPIO_PIN12)
#define GPIO_TIM1_CH4OUT_BLUE  \
    (GPIO_ALT|GPIO_AF1|GPIO_SPEED_50MHz|GPIO_PUSHPULL|GPIO_PORTE|GPIO_PIN14)

void pwm_led_init()
{
    stm32_configgpio(GPIO_TIM1_CH3OUT_RED);
    stm32_configgpio(GPIO_TIM1_CH4OUT_BLUE);

    // enable timer
    modifyreg32(STM32_RCC_APB2ENR, 0, RCC_APB2ENR_TIM1EN);

    // disable timer and configure it
    REG(STM32_TIM1_CR1) = 0;
    REG(STM32_TIM1_CR2) = 0;
    REG(STM32_TIM1_SMCR) = 0;
    REG(STM32_TIM1_DIER) = 0;
    REG(STM32_TIM1_SR) = 0;
    REG(STM32_TIM1_EGR) = GTIM_EGR_UG;
    REG(STM32_TIM1_CCMR1) = 0;
    REG(STM32_TIM1_CCMR2) = (GTIM_CCMR_MODE_PWM1 << GTIM_CCMR2_OC4M_SHIFT) |
            GTIM_CCMR2_OC4PE;
    REG(STM32_TIM1_CCMR2) |= (GTIM_CCMR_MODE_PWM1 << GTIM_CCMR2_OC3M_SHIFT) |
            GTIM_CCMR2_OC3PE;
    REG(STM32_TIM1_CCER) = GTIM_CCER_CC3NE | GTIM_CCER_CC4E;
    REG(STM32_TIM1_CNT) = 0;
    REG(STM32_TIM1_PSC) = 0;
    REG(STM32_TIM1_ARR) = 0;
    REG(STM32_TIM1_RCR) = 0;
    REG(STM32_TIM1_CCR1) = 0;
    REG(STM32_TIM1_CCR2) = 0;
    REG(STM32_TIM1_CCR3) = UINT16_MAX;
    REG(STM32_TIM1_CCR4) = UINT16_MAX;
    REG(STM32_TIM1_BDTR) = ATIM_BDTR_MOE;
    REG(STM32_TIM1_DCR) = 0;
    REG(STM32_TIM1_DMAR) = 0;

    REG(STM32_TIM1_PSC) = 10;

    // run the full span of the counter. All timers can handle uint16
    REG(STM32_TIM1_ARR) = UINT16_MAX;

    // generate an update event; reloads the counter, all registers
    REG(STM32_TIM1_EGR) = GTIM_EGR_UG | GTIM_EGR_CC4G | GTIM_EGR_CC3G;

    // enable the timer
    REG(STM32_TIM1_CR1) = GTIM_CR1_CEN;
}

static int pwm_led_set_ccr(int led, int ccr)
{
    int result = 0;

    if (led >= PWM_LED_SIZE || led < 0) {
        result = -1;
    }

    if (result == 0)
    {
        switch (led) {
            case PWM_LED_RED:
                REG(STM32_TIM1_CCR3) = ccr;
                break;

            case PWM_LED_BLUE:
                REG(STM32_TIM1_CCR4) = ccr;
                break;
        }
    }

    return result;
}

int pwm_led_set_intensity(int led, int intensity)
{
    int result = 0;

    if (led >= PWM_LED_SIZE || led < 0) {
        result = -1;
    }

    if (result == 0)
    {
        leds[led].intensity = intensity;
    }

    if (result == 0 && leds[led].active)
    {
        int i = 100 - leds[led].intensity;
        int ccr = UINT16_MAX * i / 100;
        pwm_led_set_ccr(led, ccr);
    }

    return result;
}

int pwm_led_start(int led)
{
    int result = 0;

    if (led >= PWM_LED_SIZE || led < 0) {
        result = -1;
    }

    if (result == 0)
    {
        leds[led].active = 1;
        pwm_led_set_intensity(led, leds[led].intensity);
    }

    return result;
}

int pwm_led_stop(int led)
{
    int result = 0;

    if (led >= PWM_LED_SIZE || led < 0) {
        result = -1;
    }

    if (result == 0)
    {
        leds[led].active = 0;
        pwm_led_set_ccr(led, UINT16_MAX);
    }

    return result;
}
