#ifndef ILI9341_GFX_H
#define ILI9341_GFX_H

#include "stm32f3xx_hal.h"
#include "fonts.h"

#define HORIZONTAL_IMAGE	0
#define VERTICAL_IMAGE		1

void ILI9341_DrawHollowCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color);
void ILI9341_DrawFilledCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color);
void ILI9341_DrawHollowRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color);
void ILI9341_DrawFilledRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color);
void ILI9341_DrawChar(char ch, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor);
void ILI9341_DrawText(char* str, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor);
void ILI9341_DrawImage(const uint8_t* image, uint8_t orientation);

void ILI9341_DrawChar_Scaled(char ch, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor, uint8_t size);
void ILI9341_DrawText_Scaled(char* str, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor, uint16_t size);
void DrawDataCentered_WithOffset(char* big, const uint8_t font[], uint8_t fontlarge, uint8_t offset, uint16_t color);

void DrawDataCentered2(char* big, char* smaller, char* line2, const uint8_t font[], uint8_t fontlarge, uint8_t fontsmall, uint8_t line2size);
void DrawDataInBox_TwoLines(char* big, char* smaller, char* line2, const uint8_t font[],
                            uint8_t fontlarge, uint8_t fontsmall, uint8_t line2size,
                            uint16_t boxX, uint16_t boxY, uint16_t boxW, uint16_t boxH);
void DrawSummary(char* big[4], char* small[4], char* labels[4], const uint8_t font[]);
#endif
