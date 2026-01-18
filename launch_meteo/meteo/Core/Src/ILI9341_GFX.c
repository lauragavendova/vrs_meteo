#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"

#include <string.h>

/* imprecise small delay */
__STATIC_INLINE void DelayUs(volatile uint32_t us)
{
	us *= (SystemCoreClock / 1000000);
	while (us--);
}

void ILI9341_DrawHollowCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color)
{
	int x = radius-1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	while (x >= y)
	{
		ILI9341_DrawPixel(X + x, Y + y, color);
		ILI9341_DrawPixel(X + y, Y + x, color);
		ILI9341_DrawPixel(X - y, Y + x, color);
		ILI9341_DrawPixel(X - x, Y + y, color);
		ILI9341_DrawPixel(X - x, Y - y, color);
		ILI9341_DrawPixel(X - y, Y - x, color);
		ILI9341_DrawPixel(X + y, Y - x, color);
		ILI9341_DrawPixel(X + x, Y - y, color);

		if (err <= 0)
		{
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0)
		{
			x--;
			dx += 2;
			err += (-radius << 1) + dx;
		}
	}
}

void ILI9341_DrawFilledCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color)
{

	int x = radius;
	int y = 0;
	int xChange = 1 - (radius << 1);
	int yChange = 0;
	int radiusError = 0;

	while (x >= y)
	{
		for (int i = X - x; i <= X + x; i++)
		{
			ILI9341_DrawPixel(i, Y + y,color);
			ILI9341_DrawPixel(i, Y - y,color);
		}

		for (int i = X - y; i <= X + y; i++)
		{
			ILI9341_DrawPixel(i, Y + x,color);
			ILI9341_DrawPixel(i, Y - x,color);
		}

		y++;
		radiusError += yChange;
		yChange += 2;

		if (((radiusError << 1) + xChange) > 0)
		{
			x--;
			radiusError += xChange;
			xChange += 2;
		}
	}
}

void ILI9341_DrawHollowRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color)
{
	uint16_t xLen = 0;
	uint16_t yLen = 0;
	uint8_t negX = 0;
	uint8_t negY = 0;
	float negCalc = 0;

	negCalc = X1 - X0;
	if(negCalc < 0) negX = 1;
	negCalc = 0;

	negCalc = Y1 - Y0;
	if(negCalc < 0) negY = 1;

	//DRAW HORIZONTAL!
	if(!negX)
	{
		xLen = X1 - X0;
	}
	else
	{
		xLen = X0 - X1;
	}
	ILI9341_DrawHLine(X0, Y0, xLen, color);
	ILI9341_DrawHLine(X0, Y1, xLen, color);

	//DRAW VERTICAL!
	if(!negY)
	{
		yLen = Y1 - Y0;
	}
	else
	{
		yLen = Y0 - Y1;
	}

	ILI9341_DrawVLine(X0, Y0, yLen, color);
	ILI9341_DrawVLine(X1, Y0, yLen, color);

	if((xLen > 0)||(yLen > 0))
	{
		ILI9341_DrawPixel(X1, Y1, color);
	}
}

void ILI9341_DrawFilledRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color)
{
	uint16_t xLen = 0;
	uint16_t yLen = 0;
	uint8_t negX = 0;
	uint8_t negY = 0;
	int32_t negCalc = 0;
	uint16_t X0True = 0;
	uint16_t Y0True = 0;

	negCalc = X1 - X0;
	if(negCalc < 0) negX = 1;
	negCalc = 0;

	negCalc = Y1 - Y0;
	if(negCalc < 0) negY = 1;

	if(!negX)
	{
		xLen = X1 - X0;
		X0True = X0;
	}
	else
	{
		xLen = X0 - X1;
		X0True = X1;
	}

	if(!negY)
	{
		yLen = Y1 - Y0;
		Y0True = Y0;
	}
	else
	{
		yLen = Y0 - Y1;
		Y0True = Y1;
	}

	ILI9341_DrawRectangle(X0True, Y0True, xLen, yLen, color);
}

void ILI9341_DrawChar(char ch, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor)
{
	if ((ch < 31) || (ch > 127)) return;

	uint8_t fOffset, fWidth, fHeight, fBPL;
	uint8_t *tempChar;

	fOffset = font[0];
	fWidth = font[1];
	fHeight = font[2];
	fBPL = font[3];

	tempChar = (uint8_t*)&font[((ch - 0x20) * fOffset) + 4]; /* Current Character = Meta + (Character Index * Offset) */

	/* Clear background first */
	ILI9341_DrawRectangle(X, Y, fWidth, fHeight, bgcolor);

	for (int j=0; j < fHeight; j++)
	{
		for (int i=0; i < fWidth; i++)
		{
			uint8_t z =  tempChar[fBPL * i + ((j & 0xF8) >> 3) + 1]; /* (j & 0xF8) >> 3, increase one by 8-bits */
			uint8_t b = 1 << (j & 0x07);
			if (( z & b ) != 0x00)
			{
				ILI9341_DrawPixel(X+i, Y+j, color);
			}
		}
	}
}

void ILI9341_DrawText(char* str, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor)
{
	uint8_t charWidth;			/* Width of character */
	uint8_t fOffset = font[0];	/* Offset of character */
	uint8_t fWidth = font[1];	/* Width of font */

	while (*str)
	{
		ILI9341_DrawChar(*str, font, X, Y, color, bgcolor);

		/* Check character width and calculate proper position */
		uint8_t *tempChar = (uint8_t*)&font[((*str - 0x20) * fOffset) + 4];
		charWidth = tempChar[0];

		if(charWidth + 2 < fWidth)
		{
			/* If character width is smaller than font width */
			X += (charWidth + 2);
		}
		else
		{
			X += fWidth;
		}

		str++;
	}
}

void ILI9341_DrawImage(const uint8_t* image, uint8_t orientation)
{
	if(orientation == SCREEN_HORIZONTAL_1)
	{
		ILI9341_SetRotation(SCREEN_HORIZONTAL_1);
		ILI9341_SetAddress(0,0,ILI9341_SCREEN_WIDTH,ILI9341_SCREEN_HEIGHT);
	}
	else if(orientation == SCREEN_HORIZONTAL_2)
	{
		ILI9341_SetRotation(SCREEN_HORIZONTAL_2);
		ILI9341_SetAddress(0,0,ILI9341_SCREEN_WIDTH,ILI9341_SCREEN_HEIGHT);
	}
	else if(orientation == SCREEN_VERTICAL_2)
	{
		ILI9341_SetRotation(SCREEN_VERTICAL_2);
		ILI9341_SetAddress(0,0,ILI9341_SCREEN_HEIGHT,ILI9341_SCREEN_WIDTH);
	}
	else if(orientation == SCREEN_VERTICAL_1)
	{
		ILI9341_SetRotation(SCREEN_VERTICAL_1);
		ILI9341_SetAddress(0,0,ILI9341_SCREEN_HEIGHT,ILI9341_SCREEN_WIDTH);
	}

	uint32_t counter = 0;
	for(uint32_t i = 0; i < ILI9341_SCREEN_WIDTH*ILI9341_SCREEN_HEIGHT*2/BURST_MAX_SIZE; i++)
	{
		ILI9341_WriteBuffer((uint8_t*)(image + counter), BURST_MAX_SIZE);
		counter += BURST_MAX_SIZE;

		/* DMA Tx is too fast, It needs some delay */
		DelayUs(1);
	}
}

void ILI9341_DrawChar_Scaled(char ch, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor, uint8_t size)
{
    if ((ch < 31) || (ch > 127)) return;

    uint8_t fOffset = font[0];
    uint8_t fWidth = font[1];
    uint8_t fHeight = font[2];
    uint8_t fBPL = font[3];

//    ILI9341_DrawRectangle(X, Y, fWidth * size, fHeight * size, bgcolor);

    uint8_t *tempChar = (uint8_t*)&font[((ch - 0x20) * fOffset) + 4];
    uint8_t realWidth = tempChar[0];
    ILI9341_DrawRectangle(X, Y, (realWidth + 1) * size, fHeight * size, bgcolor);

    for (int j=0; j < fHeight; j++)
    {
        for (int i=0; i < fWidth; i++)
        {
            uint8_t z = tempChar[fBPL * i + ((j & 0xF8) >> 3) + 1];
            uint8_t b = 1 << (j & 0x07);

            if ((z & b) != 0x00)
            {
                if(size <= 1)
                    ILI9341_DrawPixel(X + i, Y + j, color);
                else
                    ILI9341_DrawRectangle(X + (i * size), Y + (j * size), size, size, color);
            }
        }
    }
}

void ILI9341_DrawText_Scaled(char* str, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor, uint16_t size)
{
	uint8_t fOffset = font[0];

    while (*str)
    {
        ILI9341_DrawChar_Scaled(*str, font, X, Y, color, bgcolor, size);

        uint8_t *tempChar = (uint8_t*)&font[((*str - 0x20) * fOffset) + 4];
        uint8_t realWidth = tempChar[0];

        X += (realWidth + 1) * size;
        str++;
    }
}

void DrawDataCentered_WithOffset(char* big, const uint8_t font[], uint8_t fontlarge, uint8_t offset, uint16_t color)
{
    uint8_t fOffset = font[0];

    uint16_t width = 0;

    for (int i = 0; big[i] != '\0'; i++) {
        uint32_t charIndex = ((big[i] - 0x20) * fOffset) + 4;
        uint8_t charWidth = font[charIndex];

        width += (charWidth+1) * fontlarge;
    }

    // stred
    uint16_t xStart = (320 - width) / 2;
    uint16_t yStart = offset ;


    ILI9341_DrawText_Scaled(big, font, xStart, yStart, color, BGCOLOR, fontlarge);
}

void DrawDataCentered2(char* big, char* smaller, char* line2, const uint8_t font[], uint8_t fontlarge, uint8_t fontsmall, uint8_t line2size)
{
    uint8_t fOffset = font[0];
    uint8_t fHeight = font[2]-4;
    uint8_t spacing = 8;

    uint16_t widthLarge = 0;
    uint16_t widthSmall = 0;
    uint16_t widthLine2 = 0;

    // sirka bigger
    for (int i = 0; big[i] != '\0'; i++) {
            uint32_t charIndex = ((big[i] - 0x20) * fOffset) + 4;
            uint8_t charWidth = font[charIndex];
            widthLarge += (charWidth+1) * fontlarge;
        }

    // sirka smaller
    for (int i = 0; smaller[i] != '\0'; i++) {
            uint32_t charIndex = ((smaller[i] - 0x20) * fOffset) + 4;
            uint8_t charWidth = font[charIndex];

            widthSmall += (charWidth + 1) * fontsmall;
        }

    // sirka line2
    for (int i = 0; line2[i] != '\0'; i++) {
        uint32_t charIndex = ((line2[i] - 0x20) * fOffset) + 4;
        widthLine2 += (font[charIndex] + 1) * line2size;
    }

    // stred
    uint16_t totalWidth1 = widthLarge + widthSmall;
    uint16_t totalHeight = (fHeight * fontlarge) + spacing + (fHeight * line2size);

    uint16_t xStart1 = (320 - totalWidth1) / 2;
    uint16_t xStart2 = (320 - widthLine2) / 2;
    uint16_t yStart = (240 - totalHeight) / 2;

    // line1
    ILI9341_DrawText_Scaled(big, font, xStart1, yStart, FCOLOR, BGCOLOR, fontlarge);

    uint16_t ySmall = yStart + (fHeight * fontlarge) - (fHeight * fontsmall);
    ILI9341_DrawText_Scaled(smaller, font, xStart1 + widthLarge, ySmall, FCOLOR, BGCOLOR, fontsmall);

    // line2
    uint16_t yLine2 = yStart + (fHeight * fontlarge) + spacing;
    ILI9341_DrawText_Scaled(line2, font, xStart2, yLine2, FCOLOR, BGCOLOR, line2size);
}

void DrawDataInBox_TwoLines(char* big, char* smaller, char* line2, const uint8_t font[],
                            uint8_t fontlarge, uint8_t fontsmall, uint8_t line2size,
                            uint16_t boxX, uint16_t boxY, uint16_t boxW, uint16_t boxH)
{
    uint8_t fOffset = font[0];
    uint8_t fHeight = font[2]-4;
    uint8_t spacing = 2;

    uint16_t widthLarge = 0;
    uint16_t widthSmall = 0;
    uint16_t widthLine2 = 0;

    // sirka bigger
    for (int i = 0; big[i] != '\0'; i++) {
        uint32_t charIndex = ((big[i] - 0x20) * fOffset) + 4;
        uint8_t charWidth = font[charIndex];
        widthLarge += (charWidth + 1) * fontlarge;
    }

    // sirka
    for (int i = 0; smaller[i] != '\0'; i++) {
        uint32_t charIndex = ((smaller[i] - 0x20) * fOffset) + 4;
        uint8_t charWidth = font[charIndex]; // opravené na správny index v poli
        widthSmall += (charWidth + 1) * fontsmall;
    }

    // sirka line2
    for (int i = 0; line2[i] != '\0'; i++) {
        uint32_t charIndex = ((line2[i] - 0x20) * fOffset) + 4;
        widthLine2 += (font[charIndex] + 1) * line2size;
    }

        uint16_t totalHeight;
        if (line2[0] == '\0') {
            totalHeight = (fHeight * fontlarge);
        } else {
            totalHeight = (fHeight * fontlarge) + spacing + (fHeight * line2size);
        }

    // stred
    uint16_t totalWidth1 = widthLarge + widthSmall;


    uint16_t xStart1 = boxX + (boxW - totalWidth1) / 2;
    uint16_t xStart2 = boxX + (boxW - widthLine2) / 2;
    uint16_t yStart = boxY + (boxH - totalHeight) / 2;

    // line1
    ILI9341_DrawText_Scaled(big, font, xStart1, yStart, FCOLOR, BGCOLOR, fontlarge);

    uint16_t ySmall = yStart + (fHeight * fontlarge) - (fHeight * fontsmall);
    ILI9341_DrawText_Scaled(smaller, font, xStart1 + widthLarge, ySmall, FCOLOR, BGCOLOR, fontsmall);

    // line2
    uint16_t yLine2 = yStart + (fHeight * fontlarge) + spacing;
    ILI9341_DrawText_Scaled(line2, font, xStart2, yLine2, FCOLOR, BGCOLOR, line2size);
}

void DrawSummary(char* big[4], char* small[4], char* labels[4], const uint8_t font[])
{
    uint16_t qW = 160; // 320/2
    uint16_t qH = 120; // 240/2

    uint16_t xCoords[4] = {0, 160, 0, 160};
    uint16_t yCoords[4] = {0, 0, 120, 120};

    ILI9341_DrawHLine(0, 120, 320, FCOLOR);
    ILI9341_DrawVLine(160, 0, 240, FCOLOR);

    for (int i = 0; i < 4; i++) {
        DrawDataInBox_TwoLines(big[i], small[i], labels[i], font, 2, 1, 1,
            xCoords[i], yCoords[i], qW, qH
        );
    }
}

