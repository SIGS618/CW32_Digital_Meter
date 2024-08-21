#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include "multi_button.h"

extern struct Button btn_left;

void GPIO_Configuration(void);
void Left_SignleClick_Handler(void *btn);
void Right_SignleClick_Handler(void *btn);

#endif