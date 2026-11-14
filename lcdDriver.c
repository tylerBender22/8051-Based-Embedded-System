/*
 * Project Name: 8051 Microcontroller
 * File Name: lcdDriver.c
 * Description: TFT-LCD (ILI9341) Driver code for the 805
 *
* Author: Matthew Boeding 
 *
 * Modified by: Tyler Bender
 *
 * Modifications:
 * - Removed some unused functions
 *
 */
#include "lcdDriver.h"
#include "bmp_image.h"
#include "font.h"


__xdata uint8_t* lcd_address = (uint8_t __xdata*) LCD_ADDRESS;
/* cursor_y and cursor_x globals*/
uint16_t __idata* cursor_x = (uint16_t __idata*)(0xC2);
uint16_t __idata* cursor_y = (uint16_t __idata*)(0xC4);
/* textsize and rotation*/
uint8_t __idata* textsize = (uint8_t __idata*)(0xC6);
uint8_t __idata* rotation = (uint8_t __idata*)(0xC7);
/* text color and background */
uint16_t __idata* textcolor = (uint16_t __idata*)(0xC8);
uint16_t __idata* textbgcolor = (uint16_t __idata*)(0xCA);
/* lcd width and height */
uint16_t __idata* _width = (uint16_t __idata*)(0xCC);
uint16_t __idata* _height = (uint16_t __idata*)(0xCE);


void delay (int16_t d) 
{
	int16_t i, j = 0; /* declare for backwards compatibility with older C standards */
	for (i=0;i<d;i++) /* this is For(); loop delay used to define delay value in Embedded C*/
	{
		for (j=0;j<80;j++)
		{
		}
	}
}


inline void lcdWrite8Reg(uint8_t d) {	
	CD = 0; 		
	IOM = 1;
	*lcd_address = d;
	IOM = 0;					
}
inline void lcdWrite8Data(uint8_t d) {	
	CD = 1; 					
	IOM = 1;
	*lcd_address = d;
	IOM = 0;						
}

inline void lcdWrite8DataBlocking(uint8_t d)
{
	*lcd_address = d;
}
inline void lcdWriteRegData8(uint8_t a, uint8_t d) {
	CD = 0; 		
	IOM = 1;
	*lcd_address = a;
	IOM = 0;
	CD = 1; 					
	IOM = 1;
	*lcd_address = d;
	IOM = 0;
}

uint16_t getCursorx()
{
	return *cursor_x;
}

uint16_t getCursory()
{
	return *cursor_y;
}

uint8_t getTextSize()
{
	return *textsize;
}

void setCursorx(uint16_t x)
{
	*cursor_x = x;
}

void setCursory(uint16_t y)
{
	*cursor_y = y;
}

void lcdWriteRegData16(uint16_t a, uint16_t d){
	uint8_t hi, lo;
 	hi = (a) >> 8;
 	lo = (a);
 	lcdWrite8Reg(hi);
 	lcdWrite8Reg(lo);
  	hi = (d) >> 8;
  	lo = (d);
  	lcdWrite8Data(hi);
  	lcdWrite8Data(lo);
}


void setCursor(uint16_t x, uint16_t y){
	*cursor_x = x;
	*cursor_y = y;
}

void setTextColor(uint16_t x, uint16_t y){
	*textcolor =  x;
	*textbgcolor = y;
}



void setTextSize(uint8_t s){
	if (s > 8) return;
	*textsize = (s>0) ? s : 1 ;
}

void setRotation(uint8_t flag){
	*rotation = flag;
	switch(flag) {
		case 0:
			flag = (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR);
			*_width = TFTWIDTH;
			*_height = TFTHEIGHT;
			break;
		case 1:
			flag = (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR);
			*_width = TFTHEIGHT;
			*_height = TFTWIDTH;
			break;
		case 2:
			flag = (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR);
			*_width = TFTWIDTH;
			*_height = TFTHEIGHT;
			break;
	  	case 3:
			flag = (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR);
			*_width = TFTHEIGHT;
			*_height = TFTWIDTH;
			break;
		default:
			flag = (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR);
			*_width = TFTWIDTH;
			*_height = TFTHEIGHT;
			break;
	}
	lcdWriteRegData8(ILI9341_MEMCONTROL, flag);
}


void setAddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2){
	lcdWrite8Reg(ILI9341_COLADDRSET);
	lcdWrite8Data(x1 >> 8);
	lcdWrite8Data(x1);
	lcdWrite8Data(x2 >> 8);
	lcdWrite8Data(x2);

	lcdWrite8Reg(ILI9341_PAGEADDRSET);
	lcdWrite8Data(y1 >> 8);
	lcdWrite8Data(y1);
	lcdWrite8Data(y2 >> 8);
	lcdWrite8Data(y2);
}

void tftLcdInit(void){
	*_width = TFTWIDTH;
	*_height = TFTHEIGHT;

	lcdWrite8Reg(ILI9341_MADCTL_RGB);
	lcdWrite8Data(0x00);
	lcdWrite8Data(0x00);
	lcdWrite8Data(0x00);

	delay(200);

	lcdWrite8Reg(ILI9341_SOFTRESET);
    delay(50);
    lcdWrite8Reg(ILI9341_DISPLAYOFF);
    delay(10);

    lcdWriteRegData8(ILI9341_POWERCONTROL1, 0x23);
    lcdWriteRegData8(ILI9341_POWERCONTROL2, 0x11);
    lcdWrite8Reg(ILI9341_VCOMCONTROL1);
	lcdWrite8Data(0x3d);
	lcdWrite8Data(0x30);
	lcdWriteRegData8(ILI9341_VCOMCONTROL2, 0xaa);
    lcdWriteRegData8(ILI9341_MEMCONTROL, ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR);
    lcdWrite8Reg(ILI9341_PIXELFORMAT);
	lcdWrite8Data(0x55);lcdWrite8Data(0x00);
    lcdWriteRegData16(ILI9341_FRAMECONTROL, 0x001B);

    lcdWriteRegData8(ILI9341_ENTRYMODE, 0x07);

    lcdWrite8Reg(ILI9341_SLEEPOUT);
    delay(150);
    lcdWrite8Reg(ILI9341_DISPLAYON);
    delay(500);
	setAddress(0,0,*_width-1,*_height-1);
     /************* Start Initial Sequence ILI9341 controller **********/
}

void drawPixel(uint16_t x3,uint16_t y3,uint16_t color1)
{
	setAddress(x3,y3,x3+1,y3+1);
    lcdWrite8Reg(ILI9341_MEMORYWRITE);
	lcdWrite8Data(color1>>8);
	lcdWrite8Data(color1);
}



void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color){
	int f = 1 - r;
    	int ddF_x = 1;
    	int ddF_y = -2 * r;
    	int x = 0;
    	int y = r;


	drawPixel(x0  , y0+r, color);
    	drawPixel(x0  , y0-r, color);
    	drawPixel(x0+r, y0  , color);
    	drawPixel(x0-r, y0  , color);

    	while (x<y) {
        	if (f >= 0) {
            	y--;
            	ddF_y += 2;
            	f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void fillRect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color){
	if ((x >= *_width) || (y >= *_height))
	{
		return;
	}

	if ((x+w-1) >= *_width)
	{
		w = *_width-x;
	}

	if ((y+h-1) >= *_height)
	{
		h = *_height-y;
	}

	setAddress(x, y, x+w-1, y+h-1);


	lcdWrite8Reg(ILI9341_MEMORYWRITE);
	CD = 1;
	for(y=h; y>0; y--)
	{
		for(x=w; x>0; x--)
		{
			lcdWrite8Data(color>>8);
			lcdWrite8Data(color);
		}
	}
}

void fillScreen(){
	long len = 76800;
	int blocks = 1200;
   	uint8_t  i, hi =*textbgcolor >> 8;
        uint8_t lo = *textbgcolor;
	setAddress(0,0,*_width-1,*_height-1);

	lcdWrite8Reg(ILI9341_MEMORYWRITE);
	lcdWrite8Data(hi);
	lcdWrite8Data(lo);
	len--;
	IOM = 1;
	while(blocks--) {
      i = 16;
      do {
      		*lcd_address = hi;*lcd_address = lo;
      		*lcd_address = hi;*lcd_address = lo;
      		*lcd_address = hi;*lcd_address = lo;
      		*lcd_address = hi;*lcd_address = lo;
      } while(--i);
    }
    for(i = (char)len & 63; i--; ) {
    	*lcd_address = hi;*lcd_address = lo;
    }
    IOM = 0;

}
void drawChar(uint8_t c){
	int16_t x = *cursor_x;
	int16_t y = *cursor_y;
	uint16_t color = *textcolor;
	uint16_t bg = *textbgcolor;
	int8_t size = *textsize;
	
	int8_t i, j;
	uint8_t line;
	for (i=0; i<6; i++)
	{

		if (i == 5)
		{
			line = 0x0;
		}
		else
		{
			line = pgm_read_byte(font+(c*5)+i);
		}
		for (j = 0; j<8; j++)
		{
			if (line & 0x1)
			{
				if (size == 1) 
				{
					drawPixel(x+i, y+j, color);
				}
				else {  
					fillRect(x+(i*size), y+(j*size), size, size, color);
				}
			} else if (bg != color)
			{
				if (size == 1) 
				{
					drawPixel(x+i, y+j, bg);
				}
				else
				{ 
					fillRect(x+i*size, y+j*size, size, size, bg);
				}
			}

			line >>= 1;
		}
	}
}

void writeChar(uint8_t c)
{
	if (c == '\n')
	{
		*cursor_y += *textsize*8;
		*cursor_x  = 0;
	}
	else if (c == '\r')
	{
		
	}
	else
	{
		drawChar(c);
		*cursor_x += *textsize*6;
	}
}
void LCD_string_write(int8_t *str)
{
	int16_t i;
	for(i=0;str[i]!=0;i++)	/* Send each char of string till the NULL */
	{
		writeChar(str[i]);	/* Call transmit data function */
	}
}

uint16_t reverse(uint8_t d) {
	uint16_t rev = 0;
	uint16_t val = 0;
	while(d >= 1){

		val = d%10;
		d = d/10;
		rev = rev * 10 + val;
	}
	return rev;
}

void asciiToDec(uint8_t d) {
	uint8_t val;
	uint16_t id;
	id = reverse(d);
	while (id >= 1){

		val = id % 10;
		id = id/10;
		writeChar(val + '0');
	}
}

void asciiToHex(uint8_t d) {
	uint8_t val;
	uint8_t store[2];
	store[0] = 0;
	store[1] = 0;
	val = d & 0x0F;
	d = d >> 4;
	if (val <= 9) {
		store[0] = val + '0';
	}
	else {
		store[1] = (val%10) + 'A';
	}
	if (d <= 9) {
		store[0] = d + '0';
	}
	else {
		store[1] = (d%10) + 'A';
	}
	writeChar(store[1]);
	writeChar(store[0]);
}
