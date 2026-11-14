/*
 * Project Name: 8051 Microcontroller
 * File Name: main.c
 * Description: Main application logic for the 8051 Microcontroller.
 *
 * Author: Tyler Bender
 * Date: May 13, 2025
 *
 * Based on: Original work by Matthew Boeding 
 *
 * Modifications:
 * - Addded Custom UI Menut
 * - Dac code for playing a sine-wave.
 * - Added UART procedures.
 * - Modifications to the provided RTC code to
 * 	 allow the user to change the set time.
 *
 */

#include "main.h"

__sfr __at(0x8E) AUXR;

#define MAX_SIZE 16
#define NUM_PER_PAGE 8

__idata uint16_t buffer[MAX_SIZE];
__idata uint16_t count;
__idata uint8_t remoteKey;

inline void iowrite8(uint8_t __xdata *map_address, uint8_t d)
{
	IOM = 1;
	*map_address = d;
	IOM = 0;
}

inline uint8_t ioread8(uint8_t __xdata *map_address)
{
	uint8_t d;
	IOM = 1;
	d = *map_address;
	IOM = 0;
	return d;
}

void rtcInit(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min)
{
	uint8_t mon_ctrl = __MON_E32K_SET__ | month;
	uint8_t ctrl_a = __CTRLA_PAB_SET__;
	uint8_t ctrl_b = (__CTRLB_TE_SET__ | __CTRLB_BME_SET__);
	iowrite8((uint8_t __xdata *)(__RTC_CTRL_A_REG__), ctrl_a);
	iowrite8((uint8_t __xdata *)(__RTC_CTRL_B_REG__), ctrl_b);
	iowrite8((uint8_t __xdata *)(__RTC_MONTH_REG__), mon_ctrl);
	iowrite8((uint8_t __xdata *)(__RTC_YEAR_REG__), year);
	iowrite8((uint8_t __xdata *)(__RTC_DATE_REG__), day);
	iowrite8((uint8_t __xdata *)(__RTC_HOURS_REG__), hour);	 
	iowrite8((uint8_t __xdata *)(__RTC_MINUTES_REG__), min); 
}

uint8_t rtcBcdToAscii(uint8_t bcd)
{
	bcd = bcd & 0x0F;
	if (bcd < 10)
	{
		bcd = 0x30 + bcd;
	}
	return bcd;
}

uint8_t rtcAsciiToBcd(uint8_t ascii)
{
	if (ascii >= 0x30 && ascii < 0x3A)
	{
		ascii = ascii & 0x0F;
	}
	return ascii;
}

void rtcRegToLCD(uint8_t __xdata *reg, uint8_t mask)
{
	uint8_t rtcTemp, rtcCurrent;
	rtcCurrent = ioread8(reg);
	rtcCurrent = rtcCurrent & mask;
	rtcTemp = rtcCurrent >> 4;
	*printval = rtcBcdToAscii(rtcTemp);
	writeChar(*printval);
	*printval = rtcBcdToAscii(rtcCurrent);
	writeChar(*printval);
}

void rtcPrint()
{
	uint8_t __xdata *ptr = (uint8_t __xdata *)(__RTC_MONTH_REG__);
	rtcRegToLCD(ptr, 0x1F);
	writeChar('/');
	ptr = (uint8_t __xdata *)(__RTC_DATE_REG__);
	rtcRegToLCD(ptr, 0x3F);
	writeChar('/');
	ptr = (uint8_t __xdata *)(__RTC_YEAR_REG__);
	rtcRegToLCD(ptr, 0xFF);
	writeChar(' ');
	ptr = (uint8_t __xdata *)(__RTC_HOURS_REG__);
	rtcRegToLCD(ptr, 0x3F);
	writeChar(':');
	ptr = (uint8_t __xdata *)(__RTC_MINUTES_REG__);
	rtcRegToLCD(ptr, 0x7F);
	writeChar(':');
	ptr = (uint8_t __xdata *)(__RTC_SECONDS_REG__);
	rtcRegToLCD(ptr, 0x7F);
}

uint8_t reverse_bits(uint8_t b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

uint8_t keyDetect()
{
	uint8_t colloc, rowloc, count, size, i = 0;
	uint8_t decode = 0x10;
	uint16_t x, y;
	*exitFlag = 0;
	do
	{
		KEYPAD_PORT = 0xF0;
		colloc = KEYPAD_PORT;
		colloc &= 0xF0;
		x = getCursorx();
		y = getCursory();
		size = getTextSize();
		setCursor(320 - (6 * 20), 230);
		setTextSize(1);
		setTextColor(BLACK, WHITE);
		rtcPrint();
		setCursor(x, y);
		setTextSize(size);
		if (remoteKey)
		{
			decode = remoteKey;
			iowrite8(seg7_address, segconvert[decode]);
			remoteKey = 0;
			return decode;
		}
	} while (colloc == 0xF0);

	colloc = reverse_bits(colloc);
	KEYPAD_PORT = 0x0F;
	rowloc = KEYPAD_PORT;
	rowloc &= 0x0F;
	rowloc = reverse_bits(rowloc);
	*button = colloc | rowloc;
	count = 0;

	while (rowloc != 0x0F && count <= 60)
	{
		KEYPAD_PORT = 0x0F;
		rowloc = KEYPAD_PORT;
		delay(20);
		count++;
	}
	for (i = 0; i < 16; i++)
	{
		if (rawhex[i] == *button)
		{
			decode = i;
		}
	}
	if (count >= 60 && decode == 0x0E)
	{
		*exitFlag = 1;
	}
	return decode;
}

void keyDetectTranslate()
{
	uint8_t asc;
	uint8_t d = keyDetect();
	if (d != 0x10)
	{
		asc = ascconvert[d];
		writeChar(asc);
		iowrite8(seg7_address, segconvert[d]);
	}
}

void changeTime()
{
	fillScreen();
	setTextSize(2);
	setCursor(0, 0);
	LCD_string_write("Year(YY): ");
	uint8_t y = inByte();
	LCD_string_write("\nMonth(MM): ");
	uint8_t m = inByte();
	LCD_string_write("\nDay(DD): ");
	uint8_t d = inByte();
	LCD_string_write("\nHour([24 hour]hh): ");
	uint8_t h = inByte();
	LCD_string_write("\nMin(mm): \n");
	uint8_t mi = inByte();
	rtcInit(y, m, d, h, mi);
	fillScreen();
	setTextSize(2);
	setCursor(0, 0);
	LCD_string_write("Time set!");
	LCD_string_write("Press any key\nto continue.\n");
	while (!keyDetect())
		;
}

inline uint8_t inByte()
{
	uint8_t value = 0;
	uint8_t asc;
	uint8_t d;

	d = keyDetect();
	if (d != 0x10)
	{
		asc = ascconvert[d];
		writeChar(asc);
	}
	value = (d & 0x0F) << 4;

	d = keyDetect();
	if (d != 0x10)
	{
		asc = ascconvert[d];
		writeChar(asc);
	}
	value |= (d & 0x0F);

	return value;
}

inline uint16_t inAddress()
{
	uint8_t addHIGH = inByte();
	uint8_t addLOW = inByte();

	return ((uint16_t)addHIGH << 8) | addLOW;
}

void startTimer(void)
{
	TMOD = (TMOD & 0xF0) | 0x01; // Mode 1
	TH0 = 0x2C;
	TL0 = 0x66;
	TR0 = 1;
	while (TF0 == 0);
	TR0 = 0;
	TF0 = 0;
}

void startDac(void)
{
	fillScreen();
	setTextSize(2);
	setCursor(0, 0);
	LCD_string_write("Playing...\n");
	LCD_string_write((__code uint8_t *)return_msg);
	uint8_t val = 0;
    while (1) {
		for(uint8_t i = 0; i<0xFF; i++){
			iowite8(adc_address, sine_wave[i]);
		}
		startTimer();
        if (keyDetect() == 0x01) {
            iowrite8(adc_address, reverse_bits(0x80));
            break;
        }
    }

}

#define TILE_WIDTH 64
#define TILE_HEIGHT 64
#define TILE_SPACING 13
#define TILE_COLS 4
#define TILE_ROWS 2

// Menu tile struct in ROM
typedef struct MenuTile
{
	uint8_t menuIndex;
	const uint8_t __code *label;
	void (*action)(void);
	const struct MenuTile __code *submenu;
	uint8_t submenuSize;
	const uint8_t __code *submenuTitle;
} MenuTile;

// Main menu
__code MenuTile mainMenuItems[] = {
	{0, "Check", ramCheck, 0, 0, 0},
	{0, "Edit", ramEdit, 0, 0, 0},
	{0, "Move", ramMove, 0, 0, 0},
	{0, "Dump", ramDump, 0, 0, 0},
	{0, "Count", ramCount, 0, 0, 0},
	{0, "Find", ramFind, 0, 0, 0},
	{0, "Speaker", startDac, 0, 0, 0},
	{0, "Set Time", changeTime, 0, 0, 0},
};

static uint8_t selected = 0;

static void drawTile(const MenuTile __code *menu, uint8_t index, uint8_t highlight)
{
	uint8_t col = index % TILE_COLS;
	uint8_t row = index / TILE_COLS;

	uint16_t x = 10 + col * (TILE_WIDTH + TILE_SPACING);
	uint16_t y = 30 + row * (TILE_HEIGHT + TILE_SPACING);

	uint16_t bg = highlight ? RED : WHITE;
	uint16_t fg = highlight ? 0x0000 : 0x0000;

	fillRect(x, y, TILE_WIDTH + 2, TILE_HEIGHT + 2, 0x0000);
	fillRect(x + 1, y + 1, TILE_WIDTH, TILE_HEIGHT, bg);

	setTextSize(1);
	setTextColor(fg, bg);

	uint8_t len = 0;
	const char __code *label = menu[index].label;
	while (label[len])
		len++;

	uint16_t labelX = x + (TILE_WIDTH - len * 6) / 2 + 1;
	uint16_t labelY = y + (TILE_HEIGHT - 8) / 2 + 1;

	setCursor(labelX, labelY);
	LCD_string_write(label);
}

static void drawAllTiles(const MenuTile __code *menu, uint8_t count)
{
	for (uint8_t i = 0; i < count; i++)
	{
		drawTile(menu, i, i == selected);
	}
}

void runTileMenu(const MenuTile __code *menu, uint8_t count, const char __code *title)
{
	selected = 0;
	setTextColor(RED, WHITE); // Cyan on black
	fillScreen();
	// Draw title
	setTextSize(2);
	setCursor(10, 5);
	LCD_string_write(title);

	drawAllTiles(menu, count);

	while (1)
	{
	goHere:
		uint8_t input = keyDetect();
		uint8_t prev = selected;
		uint8_t row = selected / TILE_COLS;
		uint8_t col = selected % TILE_COLS;

		switch (input)
		{
		case 4: // left
			if (col > 0)
				selected--;
			break;
		case 6: // right
			if (col < TILE_COLS - 1 && selected + 1 < count)
				selected++;
			break;
		case 2: // up
			if (row > 0)
				selected -= TILE_COLS;
			break;
		case 8: // down
			if (row < TILE_ROWS - 1 && selected + TILE_COLS < count)
				selected += TILE_COLS;
			break;
		case 5:
		{ // select
			const MenuTile __code *item = &menu[selected];
			if (item->submenu)
			{
				runTileMenu(item->submenu, item->submenuSize, item->submenuTitle);
				return;
			}
			else if (item->action)
			{
				item->action();
			}
			setTextColor(RED, WHITE);
			fillScreen();
			setTextSize(2);
			setCursor(10, 5);
			LCD_string_write(title);

			drawAllTiles(menu, count);
			break;
		}
		case 1: // back
			return;

		default:
			goto goHere;
		}

		if (prev != selected)
		{
			drawTile(menu, prev, 0);
			drawTile(menu, selected, 1);
		}
	}
}

void serialInit(void)
{
	PCON |= 0x80;
	TMOD |= 0x20; // Timer1 mode2 (auto-reload)
	TH1 = 0xF3;	  // 9600 baud
	TL1 = 0xF3;
	TR1 = 1;
	SCON = 0x50; // UART mode1, REN=1
	IE |= 0x90;	 // EA=1, ES=1
}

void serialISR(void) __interrupt(4)
{
	if (RI)
	{
		RI = 0;
		remoteKey = SBUF;
		writeChar(remoteKey);
	}
}

void main(void)
{
	AUXR = 0x02;
	CD = CMD;
	IOM = MEM;
	*exitFlag = 0;
	iowrite8(adc_address, reverse_bits(0x80));
	serialInit();
	tftLcdInit();
	setRotation(1);
	iowrite8(seg7_address, segconvert[16]);
	runTileMenu(mainMenuItems, 8, titlePtr);
}