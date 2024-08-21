#include "bsp_adc.h"

#include <stdint.h>

#include "base_types.h"
#include "cw32f030.h"
#include "cw32f030_adc.h"
#include "cw32f030_dma.h"
#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"

ADC_ChTypeDef ADC_Ch      = Voltage_AMP2;  // 默认ADC采样通道为2.5V参考电压
const uint32_t ADC_Gain[] = {2, 23, 1, 1};

static void GPIO_Configuration(void)
{
    __RCC_GPIOA_CLK_ENABLE();  // 使能GPIOA时钟(ADC引脚在GPIOA上)

    PA00_ANALOG_ENABLE();  // 设置PA00(IN0)为模拟输入
    PA01_ANALOG_ENABLE();  // 设置PA01(IN1)为模拟输入
    PA02_ANALOG_ENABLE();  // 设置PA02(IN2)为模拟输入
    PA03_ANALOG_ENABLE();  // 设置PA03(IN3)为模拟输入
}

void ADC_Configuration(void)
{
    ADC_InitTypeDef ADC_InitStruct;          // ADC初始化结构体
    ADC_WdtTypeDef ADC_WdtStruct;            // ADC看门狗配置结构体
    ADC_SingleChTypeDef ADC_SingleChStruct;  // ADC单通道配置结构体

    __RCC_ADC_CLK_ENABLE();  // 使能ADC时钟

    /* ADC配置 */
    ADC_InitStruct.ADC_OpMode     = ADC_SingleChContinuousMode;  // 单通道连续转换模式
    ADC_InitStruct.ADC_ClkDiv     = ADC_Clk_Div16;               // ADDCLK由PCLK 16分频得到
    ADC_InitStruct.ADC_SampleTime = ADC_SampTime10Clk;           // 采样时间为10个ADC时钟周期
    ADC_InitStruct.ADC_VrefSel    = ADC_Vref_BGR1p5;             // 选择内置1.5V作为参考电压
    ADC_InitStruct.ADC_InBufEn    = ADC_BufEnable;               // 使能输入增益
    ADC_InitStruct.ADC_TsEn       = ADC_TsDisable;               // 不使能内置温度传感器
    ADC_InitStruct.ADC_DMAEn      = ADC_DmaEnable;               // ADC转换完成触发DMA
    ADC_InitStruct.ADC_Align      = ADC_AlignRight;              // ADC转换结果右对齐
    ADC_InitStruct.ADC_AccEn      = ADC_AccDisable;              // 不使能结果累加

    ADC_WdtInit(&ADC_WdtStruct);

    // 配置单通道模式
    ADC_SingleChStruct.ADC_DiscardEn  = ADC_DiscardNull;   // 覆盖未读写的转换值
    ADC_SingleChStruct.ADC_Chmux      = (uint32_t)ADC_Ch;  // 选择ADC转换通道
    ADC_SingleChStruct.ADC_InitStruct = ADC_InitStruct;    // ADC初始化结构体
    ADC_SingleChStruct.ADC_WdtStruct  = ADC_WdtStruct;     // ADC看门狗结构体

    ADC_SingleChContinuousModeCfg(&ADC_SingleChStruct);

    ADC_Enable();  // 使能ADC
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
    GPIO_Configuration();
    ADC_Configuration();
    DMA_Configuration(DMA_DstAddress, DMA_TransferCnt);
}

void ADC_SwitchCh(uint32_t DMA_DstAddress, int DMA_TransferCnt)
{
    ADC_SoftwareStartConvCmd(DISABLE);
    DMA_DeInit(CW_DMACHANNEL1);
    ADC_DeInit();
    ADC_Configuration();
    DMA_Configuration(DMA_DstAddress, DMA_TransferCnt);
    ADC_SoftwareStartConvCmd(ENABLE);
}