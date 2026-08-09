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

typedef enum
{
    LED_MODE_SINGLE =0U,
    LED_MODE_PAIR,
    LED_MODE_ALL
} LED_Mode;

static LED_Mode mode = LED_MODE_SINGLE;

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

static void led_blink_single(void)
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

static void blink_pair(const LED_BlinkParam *param1, const LED_BlinkParam *param2)
{
    if (param1 == 0 || param2 == 0 || !led_num_is_valid(param1->led_num) || !led_num_is_valid(param2->led_num))
    {
        return;
    }
    led_on(param1->led_num);
    led_on(param2->led_num);
    HAL_Delay(param1->on_ms);
    led_off(param1->led_num);
    led_off(param2->led_num);
    HAL_Delay(param1->off_ms);
}

static void led_blink_pair(void)
{
    static const LED_BlinkParam flow[] =
    {
        {LED1_ID, 250U, 250U},
        {LED2_ID, 250U, 250U},
        {LED3_ID, 250U, 250U},
        {LED4_ID, 250U, 250U}
    };

    uint8_t i;

    for (i = 0U; i < LED_COUNT; i+=2U)
    {
        blink_pair(&flow[i], &flow[(i + 1U) % LED_COUNT]);
    }
}

static void led_blink_all(void)
{
    static const LED_BlinkParam flow =
    {
        LED1_ID, 250U, 250U
    };

    uint8_t i;

    for (i = 0U; i < LED_COUNT; i++)
    {
        led_on(i);
    }
    HAL_Delay(flow.on_ms);
    for (i = 0U; i < LED_COUNT; i++)
    {
        led_off(i);
    }
    HAL_Delay(flow.off_ms);
}

void led_flow(void)
{
    uint8_t signal= 0U;

    switch (signal)
    {
        case 0:
            mode = LED_MODE_SINGLE;
            break;
        case 1:
            mode = LED_MODE_PAIR;
            break;
        case 2:
            mode = LED_MODE_ALL;
            break;
        default:
            mode = LED_MODE_SINGLE;
            break;
    }

    switch (mode)
    {
        case LED_MODE_SINGLE:
            led_blink_single();
            break;
        case LED_MODE_PAIR:
            led_blink_pair();
            break;
        case LED_MODE_ALL:
            led_blink_all();
            break;
        default:
            mode = LED_MODE_SINGLE;
            break;
    }
}
