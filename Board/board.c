/**
 * @file board.c
 * @author Lai (Laijianjun2333@outlook.com)
 * @brief Modded from lckfb.com
 * @version 1.1
 * @date 2024-08-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "board.h"

#include "cw32f030_rcc.h"
#include "cw32f030_systick.h"

// 初始化开发板, 配置时钟为64MHz
void board_init(void)
{
    // 开启HSE时钟, 开发板的高速晶振为8MHz
    RCC_HSE_Enable(RCC_HSE_MODE_OSC, 8000000, RCC_HSE_DRIVER_NORMAL,
                   RCC_HSE_FLT_CLOSE);
    // 设置HCLK的预分频系数, HCLK=64MHz
    RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
    // 设置PCLK的预分频系数, PCLK=64MHz
    RCC_PCLKPRS_Config(RCC_PCLK_DIV1);
    // 使能PLL, 配置PLL输入为HSE、倍频系数为8, 主频为64MHz
    RCC_PLL_Enable(RCC_PLLSOURCE_HSEOSC, 8000000, RCC_PLL_MUL_8);
    // 使能FLASH时钟
    RCC_AHBPeriphClk_Enable(RCC_AHB_PERIPH_FLASH, ENABLE); // __RCC_FLASH_CLK_ENABLE();
    // 配置Flash存储器读等待周期
    FLASH_SetLatency(FLASH_Latency_3);
    // 切换系统时钟到PLL
    RCC_SysClk_Switch(RCC_SYSCLKSRC_PLL);
    // 更新系统时钟频率, 64MHz
    RCC_SystemCoreClockUpdate(64000000);

    // 初始化SysTick
    SysTick->LOAD = (uint32_t)((64000000 / (1000U / uwTickFreq)) -
                               1UL); /* set reload register */
    SysTick->VAL  = 0UL;             /* Load the SysTick Counter Value */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk; /* Enable SysTick Timer */
}

/**
 * @brief 延时(单位us), 利用SysTick定时器
 *
 * @param __us
 */
void delay_us(unsigned long __us)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 38;

    // 计算需要的时钟数 = 延迟微秒数 * 每微秒的时钟数
    ticks = __us * (64000000 / 1000000);

    // 获取当前的SysTick值
    told = SysTick->VAL;

    while (1) {
        // 重复刷新获取当前的SysTick值
        tnow = SysTick->VAL;

        if (tnow != told) {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += SysTick->LOAD - tnow + told;

            told = tnow;

            // 如果达到了需要的时钟数, 就退出循环
            if (tcnt >= ticks) break;
        }
    }
}

/**
 * @brief 延时(单位ms), 利用SysTick定时器
 *
 * @param ms
 */
void delay_ms(unsigned long __ms) { delay_us(__ms * 1000); }

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, tex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
