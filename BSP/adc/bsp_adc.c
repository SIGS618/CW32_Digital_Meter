#include "bsp_adc.h"

#include "cw32f030_adc.h"
#include "cw32f030_dma.h"
#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"

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
    ADC_InitStruct.ADC_OpMode     = ADC_SingleChOneMode;  // 单通道单次转换模式
    ADC_InitStruct.ADC_ClkDiv     = ADC_Clk_Div16;        // ADDCLK由PCLK 16分频得到
    ADC_InitStruct.ADC_SampleTime = ADC_SampTime10Clk;    // 采样时间为5个ADC时钟周期
    ADC_InitStruct.ADC_VrefSel    = ADC_Vref_BGR1p5;      // 选择内置1.5V作为参考电压
    ADC_InitStruct.ADC_InBufEn    = ADC_BufEnable;        // 使能输入增益
    ADC_InitStruct.ADC_TsEn       = ADC_TsDisable;        // 不使能内置温度传感器
    ADC_InitStruct.ADC_DMAEn      = ADC_DmaEnable;        // ADC转换完成触发DMA
    ADC_InitStruct.ADC_Align      = ADC_AlignRight;       // ADC转换结果右对齐
    ADC_InitStruct.ADC_AccEn      = ADC_AccDisable;       // 不使能结果累加

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

void DMA_Configuration(uint32_t DMA_DstAddress, int DMA_TransferCnt)
{
    DMA_InitTypeDef DMA_InitStruct;   // DMA初始化结构体
    DMA_StructInit(&DMA_InitStruct);  // 使用默认参数填充DMA初始化结构体

    __RCC_DMA_CLK_ENABLE();  // 使能DMA时钟

    /* DMA配置 */
    DMA_InitStruct.DMA_Mode          = DMA_MODE_BLOCK;                  // 使用BLOCK模式, 每个数据块传输后仲裁
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_16BIT;        // 数据宽度为12bit, 需要16bit传输宽度
    DMA_InitStruct.DMA_SrcInc        = DMA_SrcAddress_Fix;              // DMA源地址固定(ADC外设地址不变)
    DMA_InitStruct.DMA_DstInc        = DMA_DstAddress_Increase;         // DMA目标地址自增
    DMA_InitStruct.DMA_TransferCnt   = DMA_TransferCnt;                 // 自动转换10次
    DMA_InitStruct.DMA_SrcAddress    = (uint32_t)(&(CW_ADC->RESULT0));  // ADC数据寄存器地址(源地址)
    DMA_InitStruct.DMA_DstAddress    = (uint32_t)DMA_DstAddress;        // 目标地址
    DMA_InitStruct.TrigMode          = DMA_HardTrig;                    // 硬件触发
    DMA_InitStruct.HardTrigSource    = DMA_HardTrig_ADC_TRANSCOMPLETE;  // ADC转换完成触发

    DMA_Init(CW_DMACHANNEL1, &DMA_InitStruct);  // 使用结构体初始化DMA
    DMA_ClearITPendingBit(DMA_IT_ALL);          // 清除DMA所有中断标志位
    DMA_ITConfig(CW_DMACHANNEL1, DMA_IT_TC,
                 ENABLE);  // 使能DMA_CHANNEL1发送完成中断

    // 使能DMA_CHANNEL1中断
    __disable_irq();
    NVIC_ClearPendingIRQ(DMACH1_IRQn);
    NVIC_EnableIRQ(DMACH1_IRQn);
    __enable_irq();

    DMA_Cmd(CW_DMACHANNEL1, ENABLE);  // 使能DMA
}

void ADC_DMA_Configuration(uint32_t DMA_DstAddress, int DMA_TransferCnt)
{
    ADC_Configuration();
    DMA_Configuration(DMA_DstAddress, DMA_TransferCnt);
}