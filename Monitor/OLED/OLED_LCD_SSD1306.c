/**
	Website: khuenguyencreator.com
	Ten Thu Vien: OLED_LCD_SSD1306
	Chuc Nang: Dieu khien man hinh SSD1306, SHT1103...
	Tac Gia: Khue Nguyen Creator
	Huong Dan Su dung: 
	- Khai b�o I2C
	- Khai b�o truoc main: SSD1306_Name SSD1306;
	- Khoi tao I2C cho LCD: SSD1306_Init(&SSD1306, &hi2c1);
	- Su dung cac ham phai truyen v�o &SSD1306
			SSD1306_Clear(&SSD1306);
			SSD1306_GotoXY(&SSD1306, 1,1);
			SSD1306_Puts(&SSD1306, "KHUE NGUYEN", &Font_11x18, SSD1306_COLOR_WHITE);
   ----------------------------------------------------------------------
 */
#include "OLED_LCD_SSD1306.h"

#define ABS(x)   ((x) > 0 ? (x) : -(x))

#define TIMEOUT 10

uint8_t I2C_WaitEvent(I2C_TypeDef* I2C, uint32_t event) {
	uint32_t startTime = TIM_GetCounter(TIM2);
	while (!I2C_CheckEvent(I2C, event)) {
		if ((TIM_GetCounter(TIM2) - startTime) >= TIMEOUT) {
			return 0;
		}
	}
	return 1;
}
				
static void SSD1306_I2C_WriteMulti(SSD1306_Name* SSD1306, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) 
{
	uint8_t TxBuff[256];
	TxBuff[0] = reg;
	for(int i = 0; i < count; i++){
		TxBuff[i+1] = data[i];
	}

	I2C_GenerateSTART(SSD1306->I2C, ENABLE);
	if (!I2C_WaitEvent(SSD1306->I2C, I2C_EVENT_MASTER_MODE_SELECT)) {
    return;
	}

	I2C_Send7bitAddress(SSD1306->I2C, address, I2C_Direction_Transmitter);
	if (!I2C_WaitEvent(SSD1306->I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
    return;
	}

	for (int i = 0; i < count + 1; i++) {
		I2C_SendData(SSD1306->I2C, TxBuff[i]);
		if (!I2C_WaitEvent(SSD1306->I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
			return;
		}
	}

	I2C_GenerateSTOP(SSD1306->I2C, ENABLE);
}

static void SSD1306_I2C_Write(SSD1306_Name* SSD1306, uint8_t address, uint8_t reg, uint8_t data)
{
	uint8_t TxBuff[2];
	TxBuff[0] = reg;
	TxBuff[1] = data;

	I2C_GenerateSTART(SSD1306->I2C, ENABLE);
	if (!I2C_WaitEvent(SSD1306->I2C, I2C_EVENT_MASTER_MODE_SELECT)) {
    return;
	}

	I2C_Send7bitAddress(SSD1306->I2C, address, I2C_Direction_Transmitter);
	if (!I2C_WaitEvent(SSD1306->I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
    return;
	}

	for (int i = 0; i < 2; i++) {
		I2C_SendData(SSD1306->I2C, TxBuff[i]);
		if (!I2C_WaitEvent(SSD1306->I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
			return;
		}
	}

	I2C_GenerateSTOP(SSD1306->I2C, ENABLE);
}

static void SSD1306_WriteCmd(SSD1306_Name* SSD1306, uint8_t Cmd)
{
	SSD1306_I2C_Write(SSD1306, SSD1306_I2C_ADDR, 0x00, Cmd);
}

//static void SSD1306_WriteData(SSD1306_Name* SSD1306, uint8_t Data)
//{
//	SSD1306_I2C_Write(SSD1306, SSD1306_I2C_ADDR, 0x40, Data);
//}


////////////////////

uint8_t SSD1306_Init(SSD1306_Name* SSD1306, I2C_TypeDef* I2C) {
	SSD1306->I2C = I2C;

	uint8_t test_data = 0x00;
	I2C_GenerateSTART(SSD1306->I2C, ENABLE);

	if (!I2C_WaitEvent(SSD1306->I2C, I2C_EVENT_MASTER_MODE_SELECT)) {
    return 0;
	}

	I2C_Send7bitAddress(SSD1306->I2C, SSD1306_I2C_ADDR, I2C_Direction_Transmitter);

	if (!I2C_WaitEvent(SSD1306->I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
    return 0;
	}
	I2C_SendData(SSD1306->I2C, test_data);

	if (!I2C_WaitEvent(SSD1306->I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
    return 0;
	}

	I2C_GenerateSTOP(SSD1306->I2C, ENABLE);

	/* A little delay */
	uint32_t p = 2500;
	while (p > 0) p--;

	/* Init LCD */
	SSD1306_WriteCmd(SSD1306, 0xAE); // Display off
	SSD1306_WriteCmd(SSD1306, 0x20); // Set Memory Addressing Mode
	SSD1306_WriteCmd(SSD1306, 0x10); // Page Addressing Mode
	SSD1306_WriteCmd(SSD1306, 0xB0); // Set Page Start Address
	SSD1306_WriteCmd(SSD1306, 0xC8); // Set COM Output Scan Direction
	SSD1306_WriteCmd(SSD1306, 0x00); // Set low column address
	SSD1306_WriteCmd(SSD1306, 0x10); // Set high column address
	SSD1306_WriteCmd(SSD1306, 0x40); // Set start line address
	SSD1306_WriteCmd(SSD1306, 0x81); // Set contrast control register
	SSD1306_WriteCmd(SSD1306, 0xFF);
	SSD1306_WriteCmd(SSD1306, 0xA1); // Set segment re-map
	SSD1306_WriteCmd(SSD1306, 0xA6); // Set normal display
	SSD1306_WriteCmd(SSD1306, 0xA8); // Set multiplex ratio
	SSD1306_WriteCmd(SSD1306, 0x3F);
	SSD1306_WriteCmd(SSD1306, 0xA4); // Output follows RAM content
	SSD1306_WriteCmd(SSD1306, 0xD3); // Set display offset
	SSD1306_WriteCmd(SSD1306, 0x00); // No offset
	SSD1306_WriteCmd(SSD1306, 0xD5); // Set display clock divide ratio
	SSD1306_WriteCmd(SSD1306, 0xF0);
	SSD1306_WriteCmd(SSD1306, 0xD9); // Set pre-charge period
	SSD1306_WriteCmd(SSD1306, 0x22);
	SSD1306_WriteCmd(SSD1306, 0xDA); // Set com pins hardware configuration
	SSD1306_WriteCmd(SSD1306, 0x12);
	SSD1306_WriteCmd(SSD1306, 0xDB); // Set VCOMH
	SSD1306_WriteCmd(SSD1306, 0x20);
	SSD1306_WriteCmd(SSD1306, 0x8D); // Set DC-DC enable
	SSD1306_WriteCmd(SSD1306, 0x14);
	SSD1306_WriteCmd(SSD1306, 0xAF); // Turn on SSD1306 panel

	SSD1306_WriteCmd(SSD1306, SSD1306_DEACTIVATE_SCROLL);

	/* Clear screen */
	SSD1306_Fill(SSD1306, SSD1306_COLOR_BLACK);

	/* Update screen */
	SSD1306_UpdateScreen(SSD1306);

	/* Set default values */
	SSD1306->CurrentX = 0;
	SSD1306->CurrentY = 0;

	/* Initialized OK */
	SSD1306->Initialized = 1;

	/* Return OK */
	return 1;
}

void SSD1306_ScrollRight(SSD1306_Name* SSD1306, uint8_t start_row, uint8_t end_row)
{
  SSD1306_WriteCmd(SSD1306,SSD1306_RIGHT_HORIZONTAL_SCROLL);  // send 0x26
  SSD1306_WriteCmd(SSD1306,0x00);  // send dummy
  SSD1306_WriteCmd(SSD1306,start_row);  // start page address
  SSD1306_WriteCmd(SSD1306,0X00);  // time interval 5 frames
  SSD1306_WriteCmd(SSD1306,end_row);  // end page address
  SSD1306_WriteCmd(SSD1306,0X00);
  SSD1306_WriteCmd(SSD1306,0XFF);
  SSD1306_WriteCmd(SSD1306,SSD1306_ACTIVATE_SCROLL); // start scroll
}


void SSD1306_ScrollLeft(SSD1306_Name* SSD1306, uint8_t start_row, uint8_t end_row)
{
  SSD1306_WriteCmd(SSD1306,SSD1306_LEFT_HORIZONTAL_SCROLL);  // send 0x26
  SSD1306_WriteCmd(SSD1306,0x00);  // send dummy
  SSD1306_WriteCmd(SSD1306,start_row);  // start page address
  SSD1306_WriteCmd(SSD1306,0X00);  // time interval 5 frames
  SSD1306_WriteCmd(SSD1306,end_row);  // end page address
  SSD1306_WriteCmd(SSD1306,0X00);
  SSD1306_WriteCmd(SSD1306,0XFF);
  SSD1306_WriteCmd(SSD1306,SSD1306_ACTIVATE_SCROLL); // start scroll
}


void SSD1306_Scrolldiagright(SSD1306_Name* SSD1306, uint8_t start_row, uint8_t end_row)
{
  SSD1306_WriteCmd(SSD1306,SSD1306_SET_VERTICAL_SCROLL_AREA);  // sect the area
  SSD1306_WriteCmd(SSD1306,0x00);   // write dummy
  SSD1306_WriteCmd(SSD1306,SSD1306_HEIGHT);

  SSD1306_WriteCmd(SSD1306,SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
  SSD1306_WriteCmd(SSD1306,0x00);
  SSD1306_WriteCmd(SSD1306,start_row);
  SSD1306_WriteCmd(SSD1306,0X00);
  SSD1306_WriteCmd(SSD1306,end_row);
  SSD1306_WriteCmd(SSD1306,0x01);
  SSD1306_WriteCmd(SSD1306,SSD1306_ACTIVATE_SCROLL);
}


void SSD1306_Scrolldiagleft(SSD1306_Name* SSD1306, uint8_t start_row, uint8_t end_row)
{
  SSD1306_WriteCmd(SSD1306,SSD1306_SET_VERTICAL_SCROLL_AREA);  // sect the area
  SSD1306_WriteCmd(SSD1306,0x00);   // write dummy
  SSD1306_WriteCmd(SSD1306,SSD1306_HEIGHT);

  SSD1306_WriteCmd(SSD1306,SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
  SSD1306_WriteCmd(SSD1306,0x00);
  SSD1306_WriteCmd(SSD1306,start_row);
  SSD1306_WriteCmd(SSD1306,0X00);
  SSD1306_WriteCmd(SSD1306,end_row);
  SSD1306_WriteCmd(SSD1306,0x01);
  SSD1306_WriteCmd(SSD1306,SSD1306_ACTIVATE_SCROLL);
}


void SSD1306_Stopscroll(SSD1306_Name* SSD1306)
{
	SSD1306_WriteCmd(SSD1306,SSD1306_DEACTIVATE_SCROLL);
}

void SSD1306_InvertDisplay(SSD1306_Name* SSD1306, int i)
{
  if(i) SSD1306_WriteCmd(SSD1306,SSD1306_INVERTDISPLAY);
  else SSD1306_WriteCmd(SSD1306,SSD1306_NORMALDISPLAY);
}
void SSD1306_DrawBitmap(SSD1306_Name* SSD1306, int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, SSD1306_COLOR_t color)
{
	int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
	uint8_t byte = 0;

	for(int16_t j=0; j<h; j++, y++)
	{
		for(int16_t i=0; i<w; i++)
		{
			if(i & 7)
			{
				byte <<= 1;
			}
			else
			{
				byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
			}
			if(byte & 0x80) SSD1306_DrawPixel(SSD1306, x+i, y, color);
		}
	}
}

void SSD1306_UpdateScreen(SSD1306_Name* SSD1306) 
{
	uint8_t m;	
	for (m = 0; m < 8; m++) {
		SSD1306_WriteCmd(SSD1306,0xB0 + m);
		SSD1306_WriteCmd(SSD1306,0x00);
		SSD1306_WriteCmd(SSD1306,0x10);
		
		/* Write multi data */
		SSD1306_I2C_WriteMulti(SSD1306, SSD1306_I2C_ADDR, 0x40, &SSD1306->SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
	}
}

void SSD1306_ToggleInvert(SSD1306_Name* SSD1306) 
{
	uint16_t i;
	/* Toggle invert */
	SSD1306->Inverted = !SSD1306->Inverted;
	/* Do memory toggle */
	for (i = 0; i < sizeof(SSD1306->SSD1306_Buffer); i++) 
	{
		SSD1306->SSD1306_Buffer[i] = ~SSD1306->SSD1306_Buffer[i];
	}
}
void SSD1306_Fill(SSD1306_Name* SSD1306, SSD1306_COLOR_t color) 
{
	/* Set memory */
	memset(SSD1306->SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306->SSD1306_Buffer));
}

void SSD1306_DrawPixel(SSD1306_Name* SSD1306, uint16_t x, uint16_t y, SSD1306_COLOR_t color) {
	if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) 
	{
		/* Error */
		return;
	}
	/* Check if pixels are inverted */
	if (SSD1306->Inverted) 
	{
		color = (SSD1306_COLOR_t)!color;
	}
	/* Set color */
	if (color == SSD1306_COLOR_WHITE) 
	{
		SSD1306->SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} 
	else 
	{
		SSD1306->SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

void SSD1306_GotoXY(SSD1306_Name* SSD1306, uint16_t x, uint16_t y)
{
	/* Set write pointers */
	SSD1306->CurrentX = x+2;
	SSD1306->CurrentY = y;
}

char SSD1306_Putc(SSD1306_Name* SSD1306, char ch, FontDef_t* Font, SSD1306_COLOR_t color) 
{
	uint32_t i, b, j;
	
	/* Check available space in LCD */
	if(SSD1306_WIDTH <= (SSD1306->CurrentX + Font->FontWidth) || SSD1306_HEIGHT <= (SSD1306->CurrentY + Font->FontHeight)) 
	{
		/* Error */
		return 0;
	}
	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) 
	{
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) 
		{
			if ((b << j) & 0x8000) 
			{
				SSD1306_DrawPixel(SSD1306, SSD1306->CurrentX + j, (SSD1306->CurrentY + i), (SSD1306_COLOR_t) color);
			} 
			else 
			{
				SSD1306_DrawPixel(SSD1306, SSD1306->CurrentX + j, (SSD1306->CurrentY + i), (SSD1306_COLOR_t)!color);
			}
		}
	}
	
	/* Increase pointer */
	SSD1306->CurrentX += Font->FontWidth;
	/* Return character written */
	return ch;
}

char SSD1306_Puts(SSD1306_Name* SSD1306, char* str, FontDef_t* Font, SSD1306_COLOR_t color) {
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (SSD1306_Putc(SSD1306, *str, Font, color) != *str) {
			/* Return error */
			return *str;
		}
		
		/* Increase string pointer */
		str++;
	}
	
	/* Everything OK, zero should be returned */
	return *str;
}


void SSD1306_DrawLine(SSD1306_Name* SSD1306, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c) {
	int16_t dx, dy, sx, sy, err, e2, i, tmp; 
	
	/* Check for overflow */
	if (x0 >= SSD1306_WIDTH) {
		x0 = SSD1306_WIDTH - 1;
	}
	if (x1 >= SSD1306_WIDTH) {
		x1 = SSD1306_WIDTH - 1;
	}
	if (y0 >= SSD1306_HEIGHT) {
		y0 = SSD1306_HEIGHT - 1;
	}
	if (y1 >= SSD1306_HEIGHT) {
		y1 = SSD1306_HEIGHT - 1;
	}
	
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1); 
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1); 
	sx = (x0 < x1) ? 1 : -1; 
	sy = (y0 < y1) ? 1 : -1; 
	err = ((dx > dy) ? dx : -dy) / 2; 

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			SSD1306_DrawPixel(SSD1306, x0, i, c);
		}
		
		/* Return from function */
		return;
	}
	
	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			SSD1306_DrawPixel(SSD1306, i, y0, c);
		}
		
		/* Return from function */
		return;
	}
	
	while (1) {
		SSD1306_DrawPixel(SSD1306, x0, y0, c);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err; 
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		} 
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		} 
	}
}

void SSD1306_DrawRectangle(SSD1306_Name* SSD1306, uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
	/* Check input parameters */
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Return error */
		return;
	}
	
	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}
	
	/* Draw 4 lines */
	SSD1306_DrawLine(SSD1306, x, y, x + w, y, c);         /* Top line */
	SSD1306_DrawLine(SSD1306, x, y + h, x + w, y + h, c); /* Bottom line */
	SSD1306_DrawLine(SSD1306, x, y, x, y + h, c);         /* Left line */
	SSD1306_DrawLine(SSD1306, x + w, y, x + w, y + h, c); /* Right line */
}

void SSD1306_DrawFilledRectangle(SSD1306_Name* SSD1306, uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
	uint8_t i;
	
	/* Check input parameters */
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Return error */
		return;
	}
	
	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}
	
	/* Draw lines */
	for (i = 0; i <= h; i++) {
		/* Draw lines */
		SSD1306_DrawLine(SSD1306, x, y + i, x + w, y + i, c);
	}
}

void SSD1306_DrawTriangle(SSD1306_Name* SSD1306, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color)
{
	/* Draw lines */
	SSD1306_DrawLine(SSD1306, x1, y1, x2, y2, color);
	SSD1306_DrawLine(SSD1306, x2, y2, x3, y3, color);
	SSD1306_DrawLine(SSD1306, x3, y3, x1, y1, color);
}


void SSD1306_DrawFilledTriangle(SSD1306_Name* SSD1306, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color) 
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
	curpixel = 0;
	
	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		SSD1306_DrawLine(SSD1306, x, y, x3, y3, color);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

void SSD1306_DrawCircle(SSD1306_Name* SSD1306, int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SSD1306_DrawPixel(SSD1306, x0, y0 + r, c);
    SSD1306_DrawPixel(SSD1306, x0, y0 - r, c);
    SSD1306_DrawPixel(SSD1306, x0 + r, y0, c);
    SSD1306_DrawPixel(SSD1306, x0 - r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawPixel(SSD1306, x0 + x, y0 + y, c);
        SSD1306_DrawPixel(SSD1306, x0 - x, y0 + y, c);
        SSD1306_DrawPixel(SSD1306, x0 + x, y0 - y, c);
        SSD1306_DrawPixel(SSD1306, x0 - x, y0 - y, c);

        SSD1306_DrawPixel(SSD1306, x0 + y, y0 + x, c);
        SSD1306_DrawPixel(SSD1306, x0 - y, y0 + x, c);
        SSD1306_DrawPixel(SSD1306, x0 + y, y0 - x, c);
        SSD1306_DrawPixel(SSD1306, x0 - y, y0 - x, c);
    }
}

void SSD1306_DrawFilledCircle(SSD1306_Name* SSD1306, int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SSD1306_DrawPixel(SSD1306, x0, y0 + r, c);
    SSD1306_DrawPixel(SSD1306, x0, y0 - r, c);
    SSD1306_DrawPixel(SSD1306, x0 + r, y0, c);
    SSD1306_DrawPixel(SSD1306, x0 - r, y0, c);
    SSD1306_DrawLine(SSD1306, x0 - r, y0, x0 + r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawLine(SSD1306, x0 - x, y0 + y, x0 + x, y0 + y, c);
        SSD1306_DrawLine(SSD1306, x0 + x, y0 - y, x0 - x, y0 - y, c);

        SSD1306_DrawLine(SSD1306, x0 + y, y0 + x, x0 - y, y0 + x, c);
        SSD1306_DrawLine(SSD1306, x0 + y, y0 - x, x0 - y, y0 - x, c);
    }
}

void OLED_DisplayData(SSD1306_Name* SSD1306, const char* label, int32_t value) {
    char buffer[32];
    int i = 0;

    // Clear the display
    SSD1306_Fill(SSD1306, SSD1306_COLOR_BLACK);

    // Write label to buffer
    while (*label && i < sizeof(buffer) - 1) {
        buffer[i++] = *label++;
    }

    // Add colon and space
    if (i < sizeof(buffer) - 2) {
        buffer[i++] = ':';
        buffer[i++] = ' ';
    }

    // Convert value to string and append to buffer
    char temp[12];
    int j = 0;
    if (value < 0) {
        buffer[i++] = '-';
        value = -value;
    }
    do {
        temp[j++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0 && j < sizeof(temp));

    while (j > 0 && i < sizeof(buffer) - 1) {
        buffer[i++] = temp[--j];
    }

    buffer[i] = '\0'; // Null-terminate the string

    // Set the cursor to the top-left corner
    SSD1306_GotoXY(SSD1306, 0, 0);

    // Display the buffer on the OLED
    SSD1306_Puts(SSD1306, buffer, &Font_7x10, SSD1306_COLOR_WHITE);

    // Update the display
    SSD1306_UpdateScreen(SSD1306);
}


void SSD1306_DisplayNumber(SSD1306_Name* SSD1306, const char* label, int number) {
    char displayBuffer[64];
    int i = 0;

    // Clear the OLED screen
    SSD1306_Clear(SSD1306);

    // Write label to displayBuffer
    while (*label && i < sizeof(displayBuffer) - 1) {
        displayBuffer[i++] = *label++;
    }

    // Add colon and space
    if (i < sizeof(displayBuffer) - 2) {
        displayBuffer[i++] = ':';
        displayBuffer[i++] = ' ';
    }

    // Convert number to string and append to displayBuffer
    char temp[12];
    int j = 0;
    if (number < 0) {
        displayBuffer[i++] = '-';
        number = -number;
    }
    do {
        temp[j++] = (number % 10) + '0';
        number /= 10;
    } while (number > 0 && j < sizeof(temp));

    while (j > 0 && i < sizeof(displayBuffer) - 1) {
        displayBuffer[i++] = temp[--j];
    }

    displayBuffer[i] = '\0'; // Null-terminate the string

    // Set cursor to (0, 0) for new text
    SSD1306_GotoXY(SSD1306, 0, 0);

    // Display the text
    SSD1306_Puts(SSD1306, displayBuffer, &Font_7x10, SSD1306_COLOR_WHITE);

    // Update the OLED screen
    SSD1306_UpdateScreen(SSD1306);
}

void SSD1306_Clear(SSD1306_Name* SSD1306)
{
	SSD1306_Fill(SSD1306, SSD1306_COLOR_BLACK);
  SSD1306_UpdateScreen(SSD1306);
}
void SSD1306_ON(SSD1306_Name* SSD1306) 
{
	SSD1306_WriteCmd(SSD1306,0x8D);  
	SSD1306_WriteCmd(SSD1306,0x14);  
	SSD1306_WriteCmd(SSD1306,0xAF);  
}
void SSD1306_OFF(SSD1306_Name* SSD1306) 
{
	SSD1306_WriteCmd(SSD1306,0x8D);  
	SSD1306_WriteCmd(SSD1306,0x10);
	SSD1306_WriteCmd(SSD1306,0xAE);  
}
