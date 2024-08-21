#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include <stdint.h>

#include "cw32f030_adc.h"

typedef enum {
    Voltage_AMP2 = ADC_ExInputCH0,
    Voltage_AMP23 = ADC_ExInputCH1,
    Current = ADC_ExInputCH2,
    Voltage_REF = ADC_ExInputCH3
} ADC_ChTypeDef;

extern const uint32_t ADC_Gain[];

void ADC_DMA_Configuration(uint32_t DMA_DstAddress, int DMA_TransferCnt);
void ADC_SwitchCh(ADC_ChTypeDef ch);

#endif