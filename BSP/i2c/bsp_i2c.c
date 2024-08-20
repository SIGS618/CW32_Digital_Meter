#include "bsp_i2c.h"

#include "base_types.h"
#include "cw32f030_gpio.h"
#include "cw32f030_i2c.h"

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
    I2C_InitStruct.I2C_Baud   = 0x13;     // 400k = (64M / (8 * (19 + 1)))
    I2C_InitStruct.I2C_FLT    = DISABLE;  // 去使能滤波器
    I2C_InitStruct.I2C_AA     = DISABLE;  // 应答时发送ACK

    I2C1_DeInit();
    I2C_Master_Init(CW_I2C1, &I2C_InitStruct);  // 初始化模块
    I2C_Cmd(CW_I2C1, ENABLE);                   // 模块使能
}

/**
 * @brief 发送i2c起始信号
 *
 * @param  I2Cx
 * @note   此函数会自检起始位的发出情况
 * @return I2C_STA
 */
I2C_STA i2c_start(I2C_TypeDef *I2Cx)
{
    I2C_GenerateSTART(I2Cx, ENABLE);
    while (I2C_GetState(I2Cx) != START_OK);
    I2C_GenerateSTART(I2Cx, DISABLE);
    return START_OK;
}

/**
 * @brief 发送i2c从机地址
 *
 * @param I2Cx
 * @param addr 从机地址
 * @param rw   读模式 | 写模式
 * @return I2C_STA
 */
I2C_STA i2c_send_addr(I2C_TypeDef *I2Cx, uint8_t addr, I2C_RWTypedef rw)
{
    I2C_Send7bitAddress(I2Cx, addr, rw);
    I2C_ClearIrq(I2Cx);
    while (I2C_GetState(I2Cx) != SLA_W_TX_OK);
    return SLA_W_TX_OK;
}

/**
 * @brief 发送8bit数据
 *
 * @param I2Cx
 * @param data 需要发送的数据
 * @return I2C_STA
 */
I2C_STA i2c_send_data(I2C_TypeDef *I2Cx, uint8_t data)
{
    I2C_SendData(I2Cx, data);
    I2C_ClearIrq(I2Cx);
    while (I2C_GetState(I2Cx) != DR_TX_OK);
    return DR_TX_OK;
}

/**
 * @brief 发送i2c停止信号
 *
 * @param  I2Cx
 * @note   此函数会自检起始位的发出情况
 * @return I2C_STA
 */
I2C_STA i2c_stop(I2C_TypeDef *I2Cx)
{
    I2C_GenerateSTOP(I2Cx, ENABLE);
    I2C_ClearIrq(I2Cx);
    while (I2C_GetState(I2Cx) != BUS_READY);
    return BUS_READY;
}