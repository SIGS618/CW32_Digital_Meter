#include "bsp_tim.h"

#include "base_types.h"
#include "cw32f030.h"
#include "cw32f030_btim.h"
#include "cw32f030_rcc.h"

void TIM_Configuration(void)
{
    BTIM_TimeBaseInitTypeDef BTIM_InitStruct;

    __RCC_BTIM_CLK_ENABLE();

    BTIM_InitStruct.BTIM_Prescaler = BTIM_PRS_DIV512;  // 16M / 512 = 31.25k
    BTIM_InitStruct.BTIM_Mode      = BTIM_Mode_TIMER;  // 定时器模式
    BTIM_InitStruct.BTIM_Period    = 3125 - 1;         // 100ms
    BTIM_TimeBaseInit(CW_BTIM1, &BTIM_InitStruct);

    BTIM_ITConfig(CW_BTIM1, BTIM_IT_OV, ENABLE);  // 使能BTIM1溢出中断

    __disable_irq();             // 禁止中断，以安全地配置NVIC
    NVIC_EnableIRQ(BTIM1_IRQn);  // 开启BTIM1中断，并关联到NVIC
    __enable_irq();              // 允许中断，恢复中断状态

    BTIM_Cmd(CW_BTIM1, ENABLE);  // 启动定时器BTIM1
}