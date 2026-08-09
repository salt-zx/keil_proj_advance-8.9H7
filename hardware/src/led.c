/**
 * @file    led.c
 * @brief   LED driver implementation.
 */
#include "led.h"
#include "stm32h7xx_hal.h"

#define LED_COUNT       4U
#define LED_ON_TIME_MS  250U
#define LED_OFF_TIME_MS 250U

#define LED_GPIO_PORT GPIOB
#define LED1_PIN      GPIO_PIN_3
#define LED2_PIN      GPIO_PIN_4
#define LED3_PIN      GPIO_PIN_5
#define LED4_PIN      GPIO_PIN_6

#define LED1_ID 0U
#define LED2_ID 1U
#define LED3_ID 2U
#define LED4_ID 3U

/*
 * signal 和模式编号保持一致：
 * 0 -> IDLE，四颗 LED 全灭
 * 1 -> SINGLE，逐个亮灭
 * 2 -> PAIR，两两亮灭
 * 3 -> ALL，一起亮灭
 */
typedef enum
{
    LED_MODE_IDLE = 0U,
    LED_MODE_SINGLE,
    LED_MODE_PAIR,
    LED_MODE_ALL
} LED_Mode;

/*
 * 一个亮灭步骤被拆成三个小阶段。
 * 非阻塞的关键就是：每次 led_flow() 只处理当前阶段，不在函数里死等。
 */
typedef enum
{
    LED_PHASE_START_ON = 0U,  /* 刚进入一个步骤：点亮当前 LED 组，并启动亮灯计时 */
    LED_PHASE_WAIT_ON,        /* 等待亮灯时间到期 */
    LED_PHASE_WAIT_OFF        /* 等待灭灯时间到期，到期后切到下一步 */
} LED_Phase;

typedef struct
{
    uint32_t start_tick;
    uint32_t period_ms;
    uint8_t running;
    uint8_t expired;
} LED_Timer;

static const uint16_t led_pin_table[LED_COUNT] =
{
    LED1_PIN,
    LED2_PIN,
    LED3_PIN,
    LED4_PIN
};

/*
 * 测试用信号变量。
 * 在 Keil 调试或临时下载验证时，可以把它依次改成 1、2、3、0。
 * 以后接真实遥控时，只需要改 get_signal() 的内部读取逻辑。
 */
static volatile uint8_t signal = 0U;

/* 下面这些 static 变量保存播放进度，return 退出后下一轮还能接着走。 */
static LED_Mode current_mode = LED_MODE_IDLE;
static uint8_t current_step = 0U;
static LED_Phase current_phase = LED_PHASE_START_ON;
static LED_Timer led_timer = {0U, 0U, 0U, 0U};

static uint8_t get_signal(void)
{
    return signal;
}

static uint32_t timer_get_tick(void)
{
    return HAL_GetTick();
}

static void timer_start(LED_Timer *timer, uint32_t now, uint32_t period_ms)
{
    if (timer == 0)
    {
        return;
    }

    timer->start_tick = now;
    timer->period_ms = period_ms;
    timer->running = 1U;
    timer->expired = 0U;
}

static void timer_stop(LED_Timer *timer)
{
    if (timer == 0)
    {
        return;
    }

    timer->running = 0U;
    timer->expired = 0U;
}

/*
 * 单次触发计时器：
 * 到期后本函数只返回一次 1，并把 timer 标记为已停止。
 * 如果不重新调用 timer_start()，它以后不会再次返回 1。
 */
static uint8_t timer_is_expired(LED_Timer *timer, uint32_t now)
{
    if (timer == 0 || timer->running == 0U || timer->expired != 0U)
    {
        return 0U;
    }

    if ((now - timer->start_tick) >= timer->period_ms)
    {
        timer->running = 0U;
        timer->expired = 1U;
        return 1U;
    }

    return 0U;
}

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

static void led_all_off(void)
{
    uint8_t i;

    for (i = 0U; i < LED_COUNT; i++)
    {
        led_off(i);
    }
}

/*
 * 点亮当前步骤对应的 LED 组：
 * SINGLE：step=0/1/2/3，对应 LED1/LED2/LED3/LED4
 * PAIR：  step=0 对应 LED1+LED2，step=1 对应 LED3+LED4
 * ALL：   只有 step=0，四颗 LED 一起亮
 */
static void led_step_on(LED_Mode mode, uint8_t step)
{
    switch (mode)
    {
        case LED_MODE_SINGLE:
            led_on(step);
            break;

        case LED_MODE_PAIR:
            led_on((uint8_t)(step * 2U));
            led_on((uint8_t)(step * 2U + 1U));
            break;

        case LED_MODE_ALL:
            led_on(LED1_ID);
            led_on(LED2_ID);
            led_on(LED3_ID);
            led_on(LED4_ID);
            break;

        default:
            break;
    }
}

static void led_step_off(LED_Mode mode, uint8_t step)
{
    switch (mode)
    {
        case LED_MODE_SINGLE:
            led_off(step);
            break;

        case LED_MODE_PAIR:
            led_off((uint8_t)(step * 2U));
            led_off((uint8_t)(step * 2U + 1U));
            break;

        case LED_MODE_ALL:
            led_all_off();
            break;

        default:
            break;
    }
}

static uint8_t led_step_count(LED_Mode mode)
{
    switch (mode)
    {
        case LED_MODE_SINGLE:
            return LED_COUNT;

        case LED_MODE_PAIR:
            return (LED_COUNT / 2U);

        case LED_MODE_ALL:
            return 1U;

        default:
            return 0U;
    }
}

static void led_reset_player(void)
{
    led_all_off();
    timer_stop(&led_timer);
    current_step = 0U;
    current_phase = LED_PHASE_START_ON;
}

/*
 * 非阻塞播放更新：
 * 这个函数不会等待 250ms，只会检查一次时间。
 * 如果时间没到就 return；main 的 while(1) 下一轮会再次调用 led_flow()。
 */
static void led_update_player(uint32_t now)
{
    uint8_t step_count;

    step_count = led_step_count(current_mode);
    if (step_count == 0U)
    {
        return;
    }

    switch (current_phase)
    {
        case LED_PHASE_START_ON:
            led_step_on(current_mode, current_step);
            timer_start(&led_timer, now, LED_ON_TIME_MS);
            current_phase = LED_PHASE_WAIT_ON;
            break;

        case LED_PHASE_WAIT_ON:
            if (!timer_is_expired(&led_timer, now))
            {
                return;
            }

            led_step_off(current_mode, current_step);
            timer_start(&led_timer, now, LED_OFF_TIME_MS);
            current_phase = LED_PHASE_WAIT_OFF;
            break;

        case LED_PHASE_WAIT_OFF:
            if (!timer_is_expired(&led_timer, now))
            {
                return;
            }

            current_step++;
            if (current_step >= step_count)
            {
                current_step = 0U;
            }

            current_phase = LED_PHASE_START_ON;
            break;

        default:
            led_reset_player();
            break;
    }
}

void led_flow(void)
{
    uint8_t new_signal;
    uint32_t now;
    LED_Mode next_mode;

    /*
     * 每轮先读 signal，所以 signal 一变，下一轮立刻切模式；
     * 不需要等当前 LED 的亮灯/灭灯计时结束。
     */
    new_signal = get_signal();
    if (new_signal > (uint8_t)LED_MODE_ALL)
    {
        new_signal = (uint8_t)LED_MODE_IDLE;
    }

    next_mode = (LED_Mode)new_signal;
    if (next_mode != current_mode)
    {
        current_mode = next_mode;
        led_reset_player();
    }

    if (current_mode == LED_MODE_IDLE)
    {
        led_all_off();
        return;
    }

    now = timer_get_tick();
    led_update_player(now);
}
