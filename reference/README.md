# 题目 3 封装思路参考

本目录是 `led_flow` 模块的封装思路参考，**不要加入 Keil 工程**。

题目 3 要求不新建文件，直接在 `led.h` / `led.c` / `main.c` 中完成。本目录只用于理解封装思路：

1. `led_flow.h`：结构体定义和 `config_apply()` 接口；
2. `led_flow.c`：`static` 配置结构体和 `config_apply()` 实现。

建议先理解 `static` 隐藏和接口暴露的方式，再把同样的思路落到原文件中，不要直接复制本目录代码。
