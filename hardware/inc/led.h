/**
 * @file    led.h
 * @brief   LED 驱动头文件。
 *          头文件放声明和宏，具体实现放在 led.c。
 */
#ifndef LED_H
#define LED_H

#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 宏定义：给 LED 使用的端口和引脚起名字 */
#define LED_GPIO_PORT GPIOB
#define LED1_PIN      GPIO_PIN_3
#define LED2_PIN      GPIO_PIN_4
#define LED3_PIN      GPIO_PIN_5
#define LED4_PIN      GPIO_PIN_6

/* 为了方便后续扩展，给 LED 编号起名字 */
#define LED1_ID 0U
#define LED2_ID 1U
#define LED3_ID 2U
#define LED4_ID 3U

/* 种子工程先提供无参数版本，只操作 LED1；题目 1 将其扩展为带编号参数版本 */
void led_on(uint8_t led_num);
void led_off(uint8_t led_num);
void led_flow(void);

#ifdef __cplusplus
}
#endif

#endif /* LED_H */
