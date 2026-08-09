/**
 * @file    led.c
 * @brief   LED driver implementation.
 */
#include "led.h"
#include "stm32h7xx_hal.h"

#define LED_COUNT 4U
#define LED_GPIO_PORT GPIOB
#define LED1_PIN      GPIO_PIN_3
#define LED2_PIN      GPIO_PIN_4
#define LED3_PIN      GPIO_PIN_5
#define LED4_PIN      GPIO_PIN_6

#define LED1_ID 0U
#define LED2_ID 1U
#define LED3_ID 2U
#define LED4_ID 3U

typedef struct
{
    uint8_t led_num;
    uint32_t on_ms;
    uint32_t off_ms;
} LED_BlinkParam;

static const uint16_t led_pin_table[LED_COUNT] =
{
    LED1_PIN,
    LED2_PIN,
    LED3_PIN,
    LED4_PIN
};

static inline uint8_t led_num_is_valid(uint8_t led_num)
{
    return (led_num < LED_COUNT);
}

static void led_on(uint8_t led_num)
{
    if (!led_num_is_valid(led_num))
    {
        return;
    }

    HAL_GPIO_WritePin(LED_GPIO_PORT, led_pin_table[led_num], GPIO_PIN_SET);
}

static void led_off(uint8_t led_num)
{
    if (!led_num_is_valid(led_num))
    {
        return;
    }

    HAL_GPIO_WritePin(LED_GPIO_PORT, led_pin_table[led_num], GPIO_PIN_RESET);
}

static void blink(const LED_BlinkParam *param)
{
    if (param == 0 || !led_num_is_valid(param->led_num))
    {
        return;
    }

    led_on(param->led_num);
    HAL_Delay(param->on_ms);
    led_off(param->led_num);
    HAL_Delay(param->off_ms);
}

void led_flow(void)
{
    static const LED_BlinkParam flow[] =
    {
        {LED1_ID, 250U, 250U},
        {LED2_ID, 250U, 250U},
        {LED3_ID, 250U, 250U},
        {LED4_ID, 250U, 250U}
    };

    uint8_t i;

    for (i = 0U; i < LED_COUNT; i++)
    {
        blink(&flow[i]);
    }
}
