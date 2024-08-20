#ifndef __OLED_H__
#define __OLED_H__

#include "font.h"

#define OLED_ADDRESS 0x78

typedef enum { COMMAND = 0, DATA } dc_typedef;

typedef struct {
    /*一共2字节，共同组成一个完整的指令码 = 数据类型字节 + 指令码本身*/
    union {
        uint8_t value[2];
        struct {
            uint8_t NC : 6;  // 无意义，仅用于填充字节
            dc_typedef
                DC : 1; /* DC为0时，表示该字节是指令, DC为1时，表示该字节是数据
                           这一部分数据会被存入Graphic Display Data
                           RAM(GGDRAM)中。
                           GGDRAM列地址指针将会在所有数据写入完成的时候被激活。
                         */
            uint8_t Co : 1;  // Co为0时，表示后续的所有字节均为数据(指令/数据)
            uint8_t CMD_DATA : 8;  // 真正发挥作用的指令/数据
        };

    } DC_byte;

} oled_frame_typedef;

typedef enum {
    OLED_COLOR_NORMAL = 0,  // 正常模式 黑底白字
    OLED_COLOR_REVERSED     // 反色模式 白底黑字
} OLED_ColorMode;

void OLED_Init();
void OLED_DisPlay_On();
void OLED_DisPlay_Off();

void OLED_NewFrame();
void OLED_ShowFrame();
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color);

void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                   OLED_ColorMode color);
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                        OLED_ColorMode color);
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                              OLED_ColorMode color);
void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                       uint8_t x3, uint8_t y3, OLED_ColorMode color);
void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                             uint8_t x3, uint8_t y3, OLED_ColorMode color);
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);
void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r,
                           OLED_ColorMode color);
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b,
                      OLED_ColorMode color);
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img,
                    OLED_ColorMode color);

void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font,
                         OLED_ColorMode color);
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str,
                           const ASCIIFont *font, OLED_ColorMode color);
void OLED_PrintString(uint8_t x, uint8_t y, char *str, const Font *font,
                      OLED_ColorMode color);

#endif  // __OLED_H__