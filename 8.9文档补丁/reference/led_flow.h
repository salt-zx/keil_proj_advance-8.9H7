/**
 * @file    led_flow.h
 * @brief   流水灯配置与状态机公共接口（参考实现）。
 */
#ifndef LED_FLOW_H
#define LED_FLOW_H

#include "stm32h7xx_hal.h"

#define LED_FLOW_STEP_COUNT 4U

typedef struct
{
    uint8_t  led_num;
    uint16_t on_ms;
    uint16_t off_ms;
} led_step_config;

typedef struct
{
    led_step_config steps[LED_FLOW_STEP_COUNT];
} led_flow_config;

void config_apply(led_flow_config config);

#endif /* LED_FLOW_H */
