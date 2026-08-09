# 题目 3 封装思路参考

本目录是 `led_flow` 模块的封装思路参考，作业里的提示也集中放在这里。题目 3 允许新建文件，可以直接按本目录结构建 `led_flow.h` / `led_flow.c`，也可以照思路在自己的文件里实现。**不建议直接复制，先理解思路再动手。**

## 一、封装思路与 include 边界

1. `led_flow.h`：对外只暴露结构体定义和公共接口（例如 `config_apply()`、流水灯运行函数），**不出现任何 `led_*` 底层声明**；
2. `led_flow.c`：`static` 配置结构体保存内部状态，实现流水灯逻辑，内部调用 `led_on()` / `led_off()`；
3. `led.h`：`led_on()` / `led_off()` 的声明保持不动，**只在需要的 `.c` 文件里 include**——`led.c` 和 `led_flow.c`；
4. `main.c`：**只 include `led_flow.h`**（和 `buzzer.h`），把原来的 `#include "led.h"` 删掉。

这样 `led_on()` 不 `static` 也进不了 `main.c`——这就是 **include 边界**：底层接口对谁可见，由头文件的 include 范围决定。

## 二、常见卡点

### 1. `led_on()` 一次只能点亮一颗 LED

"两两亮灭、一起亮灭"这类需要同时亮多颗的效果，在封装层组合出来，不要散写在 `main.c`。两种组合方式：

- 连续多次调用（简单，效果够用）：

  ```c
  /* "一起亮灭"的亮 */
  led_on(0U);
  led_on(1U);
  led_on(2U);
  led_on(3U);
  ```

- 一次传多颗：`LED1_PIN` 这类宏本身就是位掩码（`GPIO_PIN_3` 就是 `1U << 3`），用 `|` 组合后可以直接传给 `HAL_GPIO_WritePin`，一次写多颗：

  ```c
  void led_set_mask(uint16_t pin_mask, GPIO_PinState state)
  {
      HAL_GPIO_WritePin(LED_GPIO_PORT, pin_mask, state);
  }

  led_set_mask(LED1_PIN | LED2_PIN, GPIO_PIN_SET);   /* 同时点亮 LED1、LED2 */
  ```

### 2. "main 调不到"和"头文件里没有"是两回事

题目 3 考的是前者。两种手段：

- **`static`**：把 `led_on()` / `led_off()` 改成 `static`，声明只在 `led.c` 内部。注意 `static` 管的是"同一个 `.c` 文件内"——一旦把流水灯拆成 `led_flow.c`，它里面的代码就调不到 `static` 的 `led_on()` 了；
- **include 边界**：声明不动，`led.h` 只在 `led.c` / `led_flow.c` 里 include，`main.c` 只 include `led_flow.h`。**拆模块后推荐这种方式**，低层函数可以继续被多个模块使用。

## 三、常用便利接口

低层驱动除了 `led_on()` / `led_off()`，下面两个接口做多灯控制和状态翻转很方便，声明放 `led.h`、实现放 `led.c`。**两个接口的入参都是引脚掩码**，与 `HAL_GPIO_WritePin` / `HAL_GPIO_TogglePin` 的引脚参数格式一致：

```c
/* led.h */
void led_set_mask(uint16_t pin_mask, GPIO_PinState state);  /* 掩码内所有 LED 置为同一状态 */
void led_toggle_mask(uint16_t pin_mask);                    /* 掩码内所有 LED 翻转 */
```

```c
/* led.c 内部：唯一的底层写入口 */
static void led_write_mask(uint16_t pin_mask, GPIO_PinState state)
{
    HAL_GPIO_WritePin(LED_GPIO_PORT, pin_mask, state);
}

void led_set_mask(uint16_t pin_mask, GPIO_PinState state)
{
    led_write_mask(pin_mask, state);
}

void led_toggle_mask(uint16_t pin_mask)
{
    HAL_GPIO_TogglePin(LED_GPIO_PORT, pin_mask);
}
```

调用示例：

```c
led_set_mask(LED1_PIN | LED2_PIN, GPIO_PIN_SET);    /* 点亮左两颗 */
led_set_mask(LED1_PIN | LED2_PIN, GPIO_PIN_RESET);  /* 熄灭左两颗 */
led_toggle_mask(LED_ALL);                           /* 四颗全部翻转 */
```

常用的组合可以再定义成宏，灯效代码里读起来就是"两两、一起"：

```c
#define LED_LEFT  (LED1_PIN | LED2_PIN)
#define LED_RIGHT (LED3_PIN | LED4_PIN)
#define LED_ALL   (LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN)

led_set_mask(LED_ALL, GPIO_PIN_SET);   /* "一起亮灭"的亮 */
```

## 四、验收自检

做完后删掉 `main.c` 里的 `#include "led.h"` 再编译：如果报 `led_on` 未声明，说明还有地方在直接调用底层——这正是题目 3 要消灭的东西。
