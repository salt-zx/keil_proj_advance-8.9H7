/**
 * @file    led_flow.c
 * @brief   流水灯配置与状态机实现（参考实现）。
 */
#include "led_flow.h"

/* 模块内部保存当前配置，外部不能直接访问 */
static led_flow_config current_config;

void config_apply(led_flow_config config)
{
    /* 暂时采用结构体整体传参，把配置复制到 static 结构体 */
    current_config = config;
}
