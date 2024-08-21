#include "bsp_gpio.h"

#include <stdint.h>

#include "cw32f030.h"
#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"
#include "multi_button.h"

#define DEBUG

enum Button_IDs {
    btn1_id,
    btn2_id,
};

struct Button btn_left;
struct Button btn_right;

static uint8_t read_gpio_pin(uint8_t btn_id)
{
    switch (btn_id) {
        case btn1_id:
            return GPIO_ReadPin(CW_GPIOA, GPIO_PIN_10);
        case btn2_id:
            return GPIO_ReadPin(CW_GPIOB, GPIO_PIN_14);
        default:
            return NULL;
    }
}

static void Btn_Configuration(void)
{
    button_init(&btn_left, read_gpio_pin, 0, btn1_id);
    button_init(&btn_right, read_gpio_pin, 0, btn2_id);
    
    button_attach(&btn_left, SINGLE_CLICK, Left_SignleClick_Handler);
    button_attach(&btn_right, SINGLE_CLICK, Right_SignleClick_Handler);

    button_start(&btn_left);
    button_start(&btn_right);
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __RCC_GPIOA_CLK_ENABLE();
    __RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pins  = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT_PULLUP;  // 上拉, 默认为高电平, 按键按下后为低电平
    GPIO_InitStruct.IT    = GPIO_IT_NONE;            // 不使能按键中断, 状态机驱动
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;          // 慢速IO足够按键使用了
    GPIO_Init(CW_GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pins = GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

#ifdef DEBUG
    __RCC_GPIOC_CLK_ENABLE();
    GPIO_InitStruct.Pins = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init(CW_GPIOC, &GPIO_InitStruct);
    PC13_SETHIGH();
#endif

    Btn_Configuration();
}

void __WEAK Left_SignleClick_Handler(void *btn)
{
    // do nothing ...
}

void __WEAK Right_SignleClick_Handler(void *btn)
{
    // do nothing ...
}