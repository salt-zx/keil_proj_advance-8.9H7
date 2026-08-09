/**
 * @file    led.c
 * @brief   LED 驱动实现文件。
 */
#include "led.h"

/* 编号越界检查 */
static inline uint8_t led_num_is_valid(uint8_t led_num)
{
    return (led_num >= LED1_ID && led_num <= LED4_ID);
}
/* 点亮指定 LED */
void led_on(uint8_t led_num)
{
    if (!led_num_is_valid(led_num))
    {
        return;
    }

    switch (led_num)
    {
        case LED1_ID:
            HAL_GPIO_WritePin(LED_GPIO_PORT, LED1_PIN, GPIO_PIN_SET);
            break;
        case LED2_ID:
            HAL_GPIO_WritePin(LED_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
            break;
        case LED3_ID:
            HAL_GPIO_WritePin(LED_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);
            break;
        case LED4_ID:
            HAL_GPIO_WritePin(LED_GPIO_PORT, LED4_PIN, GPIO_PIN_SET);
            break;
    }
}

/* 熄灭指定 LED */
void led_off(uint8_t led_num)
{
    if (!led_num_is_valid(led_num))
    {
        return;
    }

    switch (led_num)
    {
        case LED1_ID:
            HAL_GPIO_WritePin(LED_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET);
            break;
        case LED2_ID:
            HAL_GPIO_WritePin(LED_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
            break;
        case LED3_ID:
            HAL_GPIO_WritePin(LED_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);
            break;
        case LED4_ID:
            HAL_GPIO_WritePin(LED_GPIO_PORT, LED4_PIN, GPIO_PIN_RESET);
            break;
    }
}

void led_flow(void)
{
    uint8_t i;

    for (i = LED1_ID; i <= LED4_ID; i++)
    {
        led_on(i);
        HAL_Delay(250U);
        led_off(i);
    }
}
