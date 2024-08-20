#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#include <stdint.h>

#include "cw32f030.h"

typedef enum {
    /* 主机发送模式I2C状态码 */
    START_OK    = 0x08,  // 已发送起始信号
    RESTART_OK  = 0x10,  // 已发送重复起始信号
    SLA_W_TX_OK = 0x18,  // 已发送 SLA+W, 已接收ACK
    SLA_W_TX_NG = 0x20,  // 已发送 SLA+W, 已接收NACK
    DR_TX_OK    = 0x28,  // 已发送 I2Cx_DR中的数据, 已接收ACK
    DR_TX_NG    = 0x30,  // 已发送 I2Cx_DR中的数据, 已接收NACK

    /* 主机接收模式I2C状态码 */
    SLA_R_TX_OK = 0x40,  // 已发送 SLA+R, 已接收ACK
    SLA_R_TX_NG = 0x48,  // 已发送 SLA+R, 已接收NACK
    DR_RX_OK    = 0x50,  // 已接收数据字节, ACK已返回
    DR_RX_NG    = 0x58,  // 已接收数据字节, NACK已返回

    BUS_MISS  = 0x38,  // 主机丢失仲裁
    BUS_READY = 0xF8   // I2C总线空闲
} I2C_STA;

typedef enum {
    I2C_WRITE = 0,
    I2C_READ = 1
} I2C_RWTypedef;

I2C_STA i2c_start(I2C_TypeDef *I2Cx);
I2C_STA i2c_send_addr(I2C_TypeDef *I2Cx, uint8_t addr, I2C_RWTypedef rw);
I2C_STA i2c_send_data(I2C_TypeDef *I2Cx, uint8_t data);
I2C_STA i2c_stop(I2C_TypeDef *I2Cx);

#endif