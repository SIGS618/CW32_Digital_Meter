/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 立创论坛：https://oshwhub.com/forum
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-12     LCKFB-LP    first version
 */
#include <stdint.h>
#include <stdio.h>

#include "base_types.h"
#include "board.h"
#include "bsp_uart.h"
#include "cw32f030.h"
#include "cw32f030_gpio.h"
#include "cw32f030_i2c.h"
#include "cw32f030_rcc.h"
#include "bsp_i2c.h"
#include "font.h"
#include "oled.h"

#define CONV_TIME 10

void ADC_Configuration(void);
void DMA_Configuration(void);
void I2C_Configuration(void);
uint32_t calAverage(void);

uint16_t ADC_Value[CONV_TIME] = {0};  // DMA搬运的目标地址

int32_t main(void)
{
    board_init();  // 开发板初始化
    // uart1_init(115200);  // 串口1波特率115200

    // 外设初始化
    ADC_Configuration();
    DMA_Configuration();
    I2C_Configuration();

    delay_ms(200);  // ?OLED屏幕的启动比CW32稍慢
    OLED_Init();
    OLED_NewFrame();

    float32_t conv_result = 0;
    char sprintf_buf[10];

    ADC_SoftwareStartConvCmd(ENABLE);  // 软件转换开始

    while (1) {
        // conv_result = 1.5 * calAverage() / (1 << 12) * 2;
        // sprintf(sprintf_buf, "%2.3f", conv_result);

        // OLED_PrintASCIIString(30, 24, sprintf_buf, &afont16x8,
        //                       OLED_COLOR_NORMAL);
        // OLED_PrintASCIIString(100, 24, "V", &afont16x8, OLED_COLOR_NORMAL);
        for (uint8_t i = 0; i < 64; i++) {
            OLED_NewFrame();
            OLED_DrawCircle(64, 32, i, OLED_COLOR_NORMAL);
            OLED_DrawCircle(64, 32, 2*i, OLED_COLOR_NORMAL);
            OLED_DrawCircle(64, 32, 3*i, OLED_COLOR_NORMAL);
            OLED_ShowFrame();
        }
        // OLED_ShowFrame();

        // delay_ms(500);
    }
}

void ADC_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    ADC_InitTypeDef ADC_InitStruct;  // ADC初始化结构体

    __RCC_GPIOA_CLK_ENABLE();  // 使能GPIOA时钟(ADC引脚在GPIOA上)
    __RCC_ADC_CLK_ENABLE();    // 使能ADC时钟

    /* GPIO配置 */  // ?未必是必要的
    GPIO_InitStruct.IT   = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pins = GPIO_PIN_0;
    GPIO_Init(CW_GPIOA, &GPIO_InitStruct);
    PA00_ANALOG_ENABLE();  // 设置PA00(IN0)为模拟输入

    /* ADC配置 */
    ADC_InitStruct.ADC_OpMode = ADC_SingleChOneMode;  // 单通道单次转换模式
    ADC_InitStruct.ADC_ClkDiv = ADC_Clk_Div16;  // ADDCLK由PCLK 16分频得到
    ADC_InitStruct.ADC_SampleTime =
        ADC_SampTime10Clk;  // 采样时间为5个ADC时钟周期
    ADC_InitStruct.ADC_VrefSel = ADC_Vref_BGR1p5;  // 选择内置1.5V作为参考电压
    ADC_InitStruct.ADC_InBufEn = ADC_BufEnable;  // 使能输入增益
    ADC_InitStruct.ADC_TsEn  = ADC_TsDisable;  // 不使能内置温度传感器
    ADC_InitStruct.ADC_DMAEn = ADC_DmaEnable;  // ADC转换完成触发DMA
    ADC_InitStruct.ADC_Align = ADC_AlignRight;  // ADC转换结果右对齐
    ADC_InitStruct.ADC_AccEn = ADC_AccDisable;  // 不使能结果累加

    ADC_Init(&ADC_InitStruct);  // 使用结构体初始化ADC
    // 不在ADC初始化结构体中的配置
    CW_ADC->CR1_f.DISCARD = FALSE;           // 配置数据覆盖更新
    CW_ADC->CR1_f.CHMUX   = ADC_ExInputCH0;  // 配置ADC输入通道0

    ADC_ClearITPendingAll();           // 清除中断标志
    ADC_ITConfig(ADC_IT_EOC, ENABLE);  // 使能采集完成中断
    ADC_EnableNvic(ADC_INT_PRIORITY);  // 使能NVIC中的ADC中断

    ADC_Enable();                      // 使能ADC
    ADC_SoftwareStartConvCmd(ENABLE);  // 软件转换开始
}

void DMA_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStruct;  // DMA初始化结构体
    DMA_StructInit(&DMA_InitStruct);  // 使用默认参数填充DMA初始化结构体

    __RCC_DMA_CLK_ENABLE();  // 使能DMA时钟

    /* DMA配置 */
    DMA_InitStruct.DMA_Mode =
        DMA_MODE_BLOCK;  // 使用BLOCK模式, 每个数据块传输后仲裁
    DMA_InitStruct.DMA_TransferWidth =
        DMA_TRANSFER_WIDTH_16BIT;  // 数据宽度为12bit, 需要16bit传输宽度
    DMA_InitStruct.DMA_SrcInc =
        DMA_SrcAddress_Fix;  // DMA源地址固定(ADC外设地址不变)
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Increase;  // DMA目标地址自增
    DMA_InitStruct.DMA_TransferCnt = 10;                  // 自动转换10次
    DMA_InitStruct.DMA_SrcAddress =
        (uint32_t)(&(CW_ADC->RESULT0));  // ADC数据寄存器地址(源地址)
    DMA_InitStruct.DMA_DstAddress = (uint32_t)ADC_Value;  // 目标地址
    DMA_InitStruct.TrigMode       = DMA_HardTrig;         // 硬件触发
    DMA_InitStruct.HardTrigSource =
        DMA_HardTrig_ADC_TRANSCOMPLETE;  // ADC转换完成触发

    DMA_Init(CW_DMACHANNEL1, &DMA_InitStruct);  // 使用结构体初始化DMA
    DMA_ClearITPendingBit(DMA_IT_ALL);  // 清除DMA所有中断标志位
    DMA_ITConfig(CW_DMACHANNEL1, DMA_IT_TC,
                 ENABLE);  // 使能DMA_CHANNEL1发送完成中断

    // 使能DMA_CHANNEL1中断
    __disable_irq();
    NVIC_ClearPendingIRQ(DMACH1_IRQn);
    NVIC_EnableIRQ(DMACH1_IRQn);
    __enable_irq();

    DMA_Cmd(CW_DMACHANNEL1, ENABLE);  // 使能DMA
}

void I2C_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStruct;

    __RCC_GPIOB_CLK_ENABLE();
    __RCC_I2C1_CLK_ENABLE();

    PB10_AFx_I2C1SCL();
    PB11_AFx_I2C1SDA();

    /* 引脚初始化 */
    GPIO_InitStruct.Pins  = GPIO_PIN_10 | GPIO_PIN_11;  // GPIO引脚
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;        // i2c必须开漏输出
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;            // 高速输出
    GPIO_Init(CW_GPIOB, &GPIO_InitStruct);              // 初始化

    /* i2c初始化 */
    I2C_InitStruct.I2C_BaudEn = ENABLE;   // 使能波特率计数器
    I2C_InitStruct.I2C_Baud   = 0x04;     // 400k = (16M / (8 * (4 + 1)))
    I2C_InitStruct.I2C_FLT    = DISABLE;  // 去使能滤波器
    I2C_InitStruct.I2C_AA     = DISABLE;  // 应答时发送ACK

    I2C1_DeInit();
    I2C_Master_Init(CW_I2C1, &I2C_InitStruct);  // 初始化模块
    I2C_Cmd(CW_I2C1, ENABLE);                   // 模块使能
}

uint32_t calAverage(void)
{
    uint64_t sum = 0;
    for (uint32_t i = 0; i < CONV_TIME; i++) { sum += ADC_Value[i]; }
    return sum / CONV_TIME;
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

// ADC中断服务函数
void ADC_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC_IT_EOC) != RESET) {
        ADC_ClearITPendingBit(ADC_IT_EOC);
        ADC_SoftwareStartConvCmd(ENABLE);  // 使能下一次转换
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
