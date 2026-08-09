# 8.9培训内容：C 关键字补全、枚举、结构体与数组

> 本文件是 C 语言进阶培训第一天的课程 README，工程基于当前工作区 `keil_proj_advance`。
> 前置：已完成 8.6 Git/Markdown、8.7 编译烧录、8.8 基础 C 工程操作。
> 作业单独放在 [doc/作业.md](doc/作业.md)，**本 README 只负责知识讲解**。

## 本课程教学重点

- 补齐单片机 C 中常见但之前没有展开的关键字；
- 学会用 `enum` 进行命名管理与逻辑规划，用 `struct` 组合相关数据；
- 学会使用**跳转语句**进行函数内更复杂的逻辑处理；
- 理解 `const` 表、宏函数和 `inline` 函数在空间与时间上的权衡；
- 建立 `.h` / `.c` 模块边界、`static` 封装和分层意识。

## 硬件资源

| 外设 | 引脚 | 说明 |
| --- | --- | --- |
| LED1 | PB3 | 高电平点亮 |
| LED2 | PB4 | 高电平点亮 |
| LED3 | PB5 | 高电平点亮 |
| LED4 | PB6 | 高电平点亮 |
| 蜂鸣器 | PB0 | 高电平响 |

## 工程目录结构

```text
keil_proj_advance/
├── Core/                     # CubeMX 生成的主程序
├── Drivers/                  # HAL 库与 CMSIS
├── hardware/
│   ├── inc/
│   │   ├── led.h             # LED 公共接口、枚举与配置结构体
│   │   └── buzzer.h          # 蜂鸣器公共接口与模式枚举
│   └── src/
│       ├── led.c             # LED 表驱动实现
│       └── buzzer.c          # 蜂鸣器实现
├── doc/
│   └── 作业.md          # 当天作业
├── MDK-ARM/
│   └── keil_proj_advance.uvprojx
└── keil_proj_advance.ioc
```

## 程序运行流程

```text
复位
  -> HAL_Init
  -> SystemClock_Config
  -> MX_GPIO_Init
  -> led_demo_init
  -> while (1)
      -> led_demo_update
```

`main.c` 中的调用关系：

```c
led_demo_init();

while (1)
{
    led_demo_update();
}
```

## 本工程覆盖的 C 语法

基础教程已经讲过变量、函数、`#define`、`for/while/if/switch` 等，今天不再重复。下面补齐嵌入式 C 中更常出现的关键字，并展开枚举、结构体、数组和 `const` 表。

### 关键字补全

| 分类 | 关键字 | 本课要求 |
| --- | --- | --- |
| 存储类 | `static`、`extern`、`register` | 控制变量或函数的可见范围、生命周期和存储位置 |
| 类型与限定 | `const`、`volatile`、`typedef`、`signed`、`unsigned`、`sizeof` | 定义类型、修饰类型、查询类型大小 |
| 控制流 | `break`、`continue`、`return` | 控制循环和函数的中途退出 |
| 组合类型 | `enum`、`struct`、`union` | 创建新的数据类型 |

#### 存储类：static、extern、register

**`static` 是单片机工程里最常见的封装工具**，有三种常见用法：

- 修饰函数时，表示这个函数只能在当前 `.c` 文件内使用；
- 修饰局部变量时，表示变量在函数多次调用之间保持上次的值；
- 修饰全局变量时，表示变量只能被当前 `.c` 文件访问。

例如 `buzzer_alarm.c` 的蜂鸣器报警模块就可以写成：

```c
static uint16_t beep_count = 0U;

static void buzzer_on(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}

static void buzzer_off(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
}

void alarm_count_request(uint16_t count)
{
    beep_count = count;
}

void alarm_func(void)
{
    while (beep_count > 0U)
    {
        buzzer_on();
        HAL_Delay(200U);
        buzzer_off();
        HAL_Delay(200U);
        beep_count--;
    }
}
```

这里 `buzzer_on()`、`buzzer_off()` 和 `beep_count` 都是 `static`，外部不能直接调用或修改；对外只暴露两个接口：

```c
/* buzzer_alarm.h */
void alarm_count_request(uint16_t count);
void alarm_func(void);
```

封装的思想是：**调用方只依赖稳定接口，不依赖内部细节。** 调用方只需要知道 `alarm_count_request(3U)` 表示“请求响 3 次”，`alarm_func()` 表示“执行报警”；不需要知道蜂鸣器接在 PB0、响多久、次数存在哪个变量。以后要改引脚、改延时，或者把阻塞循环改成状态机，只需要改 `buzzer_alarm.c`，调用方不用动。**工程中的原则：模块只通过 `.h` 暴露公共 API，内部符号尽量用 `static` 藏起来。**

`extern` 用于声明一个定义在其他 `.c` 文件中的变量或函数。函数声明默认就有 `extern` 含义，所以头文件里通常不写；变量跨文件使用时才显式写：

```c
/* app.h */
extern uint32_t app_tick;

/* app.c */
uint32_t app_tick = 0U;
```
**不要滥用 `extern` 变量。** 一旦用 `extern` 暴露一个全局变量，任何 `.c` 文件都能读写它，调用关系会变得隐蔽，多个模块之间也容易互相踩到数据。工程里更推荐的做法是：把变量留在定义它的模块内，通过函数接口读写；只有确实需要跨模块共享时才使用 `extern`。

`register` 是向编译器建议“把这个变量放进寄存器”，现代编译器会自动决定，单片机代码里基本不需要写。

#### 类型与限定：const、volatile、typedef、signed、unsigned、sizeof

`const` 表示“通过这个名字不能修改值”，适合定义只读配置：

```c
const uint16_t default_delay_ms = 250U;
```

`const` 变量有类型、可能占用存储，适合数组和结构体；`#define` 只是预处理阶段的文本替换，适合简单常量。后面要讲的 `static const` 表就是 `const` 在工程里最重要的用法。

**`volatile` 告诉编译器“这个变量的值可能在当前代码流之外被修改”**，典型场景是中断和主循环共享的变量：

```c
volatile uint8_t tick_ms = 0U;   /* 中断里修改，主循环读取 */
```

如果没有 `volatile`，编译器可能把变量优化到寄存器里，导致主循环读不到中断更新的值。目前了解即可。

`typedef` 给已有类型起别名，让类型名更短、更统一：

```c
typedef struct
{
    uint16_t pin;
    uint16_t on_ms;
    uint16_t off_ms;
} led_config;
```

`typedef` 创建的是别名，不是新的类型。命名风格跟随团队现有工程即可，本教程不额外规定。

`signed` 和 `unsigned` 表示整数是否有符号。`uint8_t`、`int32_t` 这些标准类型已经写明了符号性；自己写 `char`、`short`、`int` 时要小心。无符号数减到 0 后再减会发生回绕，例如：

```c
uint8_t i = 0U;
i = i - 1U;   /* i 变成 255U，而不是 -1 */
```

**`sizeof` 是编译期运算符，不是函数**，用来查询变量或类型占用的字节数：

```c
uint16_t delay_ms = 250U;

uint32_t size_var  = sizeof(delay_ms);    /* sizeof(变量) */
uint32_t size_type = sizeof(uint16_t);    /* sizeof(类型) */
uint32_t size_int  = sizeof(int);         /* sizeof(内置类型) */
```

以上三种写法都常见：变量写法在数组、结构体上更直观，类型写法适合不创建变量时使用。在 H723 上，`sizeof(uint16_t)` 通常为 2，`sizeof(int)` 通常为 4。`sizeof` 在编译期计算，不会在运行时产生额外开销；后面 `const` 表小节会用 `sizeof(数组) / sizeof(数组[0])` 自动得到元素个数。

#### 跳转语句：break、continue、return

`for`、`while`、`if`、`switch` 已在基础教程讲过，今天补充三个控制流关键字。

`break` 用于跳出最近一层循环或 `switch`。例如循环到指定编号时提前停止：

```c
for (i = 0U; i < LED_COUNT; i++)
{
    if (i == 2U)
    {
        break;
    }

    led_on((led_id)i);
    HAL_Delay(100U);
    led_off((led_id)i);
}
```

`continue` 用于跳过本次循环剩余代码，直接进入下一次循环：

```c
for (i = 0U; i < LED_COUNT; i++)
{
    if (i == 2U)
    {
        continue;
    }
    led_on((led_id)i);
    HAL_Delay(100U);
    led_off((led_id)i);
}
```

`return` 用于结束当前函数并回到调用处。`void` 函数可以直接写 `return;`，例如非法参数保护：

```c
void led_on(led_id id)
{
    if (id >= LED_COUNT)
    {
        return;
    }

    switch (led_id):
    case LED_1 :HAL_GPIO_WritePin(GPIO_LED_PORT, GPIO_PIN_3, GPIO_PIN_SET); break;
    case LED_2 :HAL_GPIO_WritePin(GPIO_LED_PORT, GPIO_PIN_4, GPIO_PIN_SET); break;
    ...
}
```
> 函数内多 `if` 的结构下,记得为每一个 `if` 出口配上return。

#### 组合类型：enum、struct、union

`enum` 和 `struct` 在下面有专门小节，这里先说明它们在关键字体系中的位置：

- `enum` 给一组整数常量命名，适合状态、模式、命令；
- `struct` 把不同类型的数据组合成一个对象；
- `union` 让多个成员共享同一块内存，大小由最大的成员决定，适合寄存器或协议解析场景。

`union` 只建立概念，不深入使用：

```c
typedef union
{
    uint32_t word;
    uint8_t bytes[4];
} reg_view;
```

### 枚举

```c
typedef enum
{
    LED1 = 0,
    LED2,
    LED3,
    LED4,
    LED_COUNT
} led_id;
```

要点：

- 枚举值默认从 0 开始递增，也可以显式赋值；
- 用枚举给状态、模式、命令命名，代码里不再出现含义不明的数字；
- **枚举本质上还是整数**，底层大小由编译器决定，做寄存器级映射时不能想当然；
- **`LED_COUNT` 放在最后**，可以作为循环上限和非法编号检查；const 表的实际长度可以用 `sizeof` 自动得到；
- 枚举常量是编译期整数常量，可以用作数组下标、`switch` 的 `case` 和函数参数；
- **定义与实例要分清**：`typedef enum { ... } led_id;` 是类型定义，通常放在 `.h`；`led_id current_led = LED1;` 才是实例定义，放在 `.c`。

### 结构体

回到蜂鸣器报警系统。原来 `alarm_func()` 里两个延时是写死的：

```c
buzzer_on();
HAL_Delay(200U);
buzzer_off();
HAL_Delay(200U);
```

如果希望“响多久、停多久、响几次”都可以配置，就可以把这些参数放进一个结构体：

```c
typedef struct
{
    uint16_t on_ms;
    uint16_t off_ms;
    uint16_t repeat_count;
} alarm_config;
```

类型定义通常放在 `.h`，真正使用的实例放在 `.c`：

```c
static alarm_config alarm_cfg = { 250U, 150U, 0U };
```

`alarm_func()` 里的固定延时就可以改成：

```c
void alarm_func(void)
{
    while (alarm_cfg.repeat_count > 0U)
    {
        buzzer_on();
        HAL_Delay(alarm_cfg.on_ms);
        buzzer_off();
        HAL_Delay(alarm_cfg.off_ms);
        alarm_cfg.repeat_count--;
    }
}
```

要点：

- 结构体把一组相关数据放在一起，例如一次报警的响延时、停延时和重复次数；
- 访问成员用 `.`，结构体可以作为函数参数整体传入；
- **结构体数组可以描述一组同类对象**，例如 4 颗 LED 的配置；
- **定义与实例要分清**：`typedef struct { ... } alarm_config;` 是类型定义，通常放在 `.h`；`static alarm_config alarm_cfg = {...};` 是实例定义，放在 `.c`；
- 结构体整体传参会复制整块数据，等后期课程学过指针后，再学如何传地址避免复制；
- **封装和结构体带来灵活性，也带来复杂度**：调用方需要理解字段含义，模块内部也要维护实例；如果一个参数只有一个固定值且永远不会变，直接写常量更简单，不要为了封装而封装。

### 宏函数与 inline 函数

宏函数和 `inline` 函数都常用于“用空间换时间”，也是后面 `const` 表会用到的写法。

宏函数在预处理阶段做文本替换，没有函数调用开销，但也没有类型检查，参数必须加括号：

```c
#define IS_VALID_LED(id) ((id) < LED_COUNT)
```

`inline` 函数保留函数的类型检查和可读性，同时给编译器“尽量内联”的建议：

```c
static inline bool is_valid_led(led_id id)
{
    return (id < LED_COUNT);
}
```

**`inline` 只是建议**，最终是否真正内联由编译器决定；宏函数虽然快，但参数带副作用时容易被多次求值，所以工程里优先考虑普通函数或 `inline` 函数。

### 数组与 const 表（掌握思想即可）

```c
static const led_config led_table[] =
{
    { GPIO_PIN_3, 250U, 250U },
    { GPIO_PIN_4, 250U, 250U },
    { GPIO_PIN_5, 250U, 250U },
    { GPIO_PIN_6, 250U, 250U }
};

#define LED_TABLE_COUNT (sizeof(led_table) / sizeof(led_table[0]))
```

要点：

- 数组是“相同类型的一组数据”，配合循环可以批量处理；
- `const` 表适合存放不会变化的数据，例如引脚配置和模式参数；
- **表驱动让 `led_on()` / `led_off()` 从多个 `case` 变成一次查表**；
- **下标越界不会在编译期报错**，是嵌入式常见 bug 来源，因此查表前最好检查下标范围；
- 查表是一种典型的**空间换时间**：把运行时计算或分支判断提前变成一张表，执行时直接按下标取结果，代价是表格本身要占用 Flash；
- `const` 表放在只读存储区，不会占用宝贵的 RAM，这也是嵌入式里愿意用表格的原因；
- 表长度用 `sizeof(led_table) / sizeof(led_table[0])` 自动得到，增减表项后不用手动改，这就是前面 `sizeof` 提到的主要用途。

在函数层看，查表的写法会比 `switch` 更短；查表前用前面讲过的 `IS_VALID_LED()` 宏函数检查下标：

```c
void led_on(led_id id)
{
    if (!IS_VALID_LED(id))
    {
        return;
    }

    led_write_pin(led_table[id].pin, GPIO_PIN_SET);
}
```

前面讲的宏函数、`inline` 函数和这里的 `const` 表可以放在一起对比：

| 方式 | 处理阶段 | 类型检查 | 适用场景 |
| --- | --- | --- | --- |
| `const` 表 | 数据存放在 Flash，运行时查表 | 有 | 批量配置、查找、避免分支 |
| 宏函数 | 预处理文本替换 | 无 | 简单表达式、位操作、合法性判断 |
| `inline` 函数 | 编译期展开候选 | 有 | 短小且频繁调用的函数 |

**`inline` 只是建议**，最终是否真正内联由编译器决定；宏函数虽然快，但参数带副作用时容易被多次求值，所以工程里优先考虑普通函数或 `inline` 函数。

### 状态机

状态机可以理解成“程序当前处于哪个阶段，以及什么时候切换到下一个阶段”。它的核心概念可以分成四部分：

- **enter（进入）**：进入某个状态时执行一次的动作，例如点亮 LED、初始化变量；
- **run（运行）**：停留在该状态期间反复执行的动作，例如保持 LED 状态、更新延时；
- **exit（退出）**：离开该状态前执行一次的动作，例如熄灭 LED、清空变量；
- **switch（迁移）**：判断条件是否满足，决定是否切换到其他状态。

**这四部分只是理解状态机的框架，不是代码模板。** 实际工程里，简单状态机往往只需要一个状态变量加 `switch(state)`，甚至几个 `if` 就够了；enter/exit 这种结构更多出现在状态数量多、每个状态动作复杂的模块里。先掌握“状态 + 迁移”的思路，代码实现可以保持简单。`led_demo_update()` 就是根据当前状态决定下一步动作，状态机是后续机器人控制代码里非常常见的结构。

## 作业与参考资料

- 当天作业：[doc/作业.md](doc/作业.md)
- 上一阶段工程 README：`keil_proj_demo/README.md`
- [C语言语法基础网课](https://www.bilibili.com/video/BV1qCSkY7EyD?p=19)，推荐带着问题去看

## 暂时不展开的内容

指针、数组、函数指针、队列、位运算，这些留到后续课程。
