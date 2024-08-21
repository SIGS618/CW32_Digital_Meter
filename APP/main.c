#include <stdint.h>
#include <stdio.h>

#include "base_types.h"
#include "board.h"
#include "bsp_adc.h"
#include "bsp_gpio.h"
#include "bsp_i2c.h"
#include "bsp_tim.h"
#include "bsp_uart.h"
#include "cw32f030.h"
#include "cw32f030_gpio.h"
#include "multi_button.h"
#include "oled.h"

#define CNV_TIMES 10

void calAverage(void);

uint16_t ADC_Value[CNV_TIMES] = {0};  // DMA搬运的目标数组(地址)
float32_t cnv_value;                  // 计算得到的电压
char sprintf_buf[10];                 // 电压/电流值字符串

uint8_t flag_NewFrame = 0;  // 判断是否要刷新屏幕

int32_t main(void)
{
    board_init();  // 开发板初始化

    // 外设初始化
    GPIO_Configuration();
    ADC_DMA_Configuration((uint32_t)ADC_Value, CNV_TIMES);
    I2C_Configuration();
    TIM_Configuration();

    delay_ms(200);    // OLED屏幕的启动比CW32稍慢
    OLED_Init();      // OLED初始化
    OLED_NewFrame();  // 清屏

    ADC_SoftwareStartConvCmd(ENABLE);  // 软件使能ADC转换

    while (1) {
        // for (uint8_t i = 0; i < 64; i++) {
        //     OLED_NewFrame();
        //     OLED_DrawCircle(64, 32, i, OLED_COLOR_NORMAL);
        //     OLED_DrawCircle(64, 32, 2 * i, OLED_COLOR_NORMAL);
        //     OLED_DrawCircle(64, 32, 3 * i, OLED_COLOR_NORMAL);
        //     OLED_ShowFrame();
        // }

        if (flag_NewFrame) {
            OLED_NewFrame();
            sprintf(sprintf_buf, "%2.3f", cnv_value);
            OLED_PrintASCIIString(30, 24, sprintf_buf, &afont16x8, OLED_COLOR_NORMAL);
            OLED_PrintASCIIString(100, 24, "V", &afont16x8, OLED_COLOR_NORMAL);
            OLED_ShowFrame();
            flag_NewFrame = 0;
        }
    }
}

void calAverage(void)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < CNV_TIMES; i++) sum += ADC_Value[i];
    cnv_value = 1.5 * sum / CNV_TIMES / (1 << 12) * ADC_Gain[ADC_Ch];
}

// DMA通道1中断服务函数
void DMACH1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA_IT_TC1)) {  // 判断中断来源
        DMA_ClearITPendingBit(DMA_IT_TC1);
        CW_DMACHANNEL1->CNT     = 0x10010;              // 重置CNT计数
        CW_DMACHANNEL1->DSTADDR = (uint32_t)ADC_Value;  // 重置目的地址
        DMA_Cmd(CW_DMACHANNEL1, ENABLE);                // 重新使能DMA
    }
}

// BTIM1中断服务函数 100ms
void BTIM1_IRQHandler(void)
{
    if (BTIM_GetITStatus(CW_BTIM1, BTIM_IT_OV)) {  // 判断中断来源
        calAverage();
        flag_NewFrame = 1;

        // 换挡
        if (ADC_Ch == Voltage_AMP2 && cnv_value > 2.99) {
            ADC_Ch = Voltage_AMP23;
            ADC_SwitchCh((uint32_t)ADC_Value, CNV_TIMES);
        } else if (ADC_Ch == Voltage_AMP23 && cnv_value < 2.8) {
            ADC_Ch = Voltage_AMP2;
            ADC_SwitchCh((uint32_t)ADC_Value, CNV_TIMES);
        }

        BTIM_ClearITPendingBit(CW_BTIM1, BTIM_IT_OV);  // 清除中断标志位
    }
}

// BTIM2中断服务函数 5ms
void BTIM2_IRQHandler(void)
{
    if (BTIM_GetITStatus(CW_BTIM2, BTIM_IT_OV)) {  // 判断中断来源
        button_ticks();
        BTIM_ClearITPendingBit(CW_BTIM2, BTIM_IT_OV);  // 清除中断标志位
    }
}

void OLED_Send(uint8_t *data, uint8_t len)
{
    i2c_start(CW_I2C1);
    i2c_send_addr(CW_I2C1, OLED_ADDRESS, I2C_WRITE);
    for (uint8_t i = 0; i < len; i++)
        i2c_send_data(CW_I2C1, data[i]);
    i2c_stop(CW_I2C1);
}

void Left_SignleClick_Handler(void *btn)
{
    ADC_Ch = Voltage_AMP23;
    ADC_SwitchCh((uint32_t)ADC_Value, CNV_TIMES);
}

void Right_SignleClick_Handler(void *btn)
{
    ADC_Ch = Current;
    ADC_SwitchCh((uint32_t)ADC_Value, CNV_TIMES);
    PC13_TOG();
}
