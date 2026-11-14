/*
 * Project Name: 8051 Microcontroller
 * File Name: ramDriver.c
 * Description: Memory Drivers for the 8051 Microcontroller.
 *
 * Author: Tyler Bender
 * Date: May 13, 2025
 *
 */

#include "ramDriver.h"

uint8_t *byte_to_hexstr(uint8_t byte)
{
    static char str[3];
    const char hex_digits[] = "0123456789ABCDEF";

    str[0] = hex_digits[(byte >> 4) & 0x0F]; 
    str[1] = hex_digits[byte & 0x0F];        
    str[2] = '\0';                          

    return str;
}

uint8_t *word_to_hexstr(uint16_t word)
{
    static char str[5]; 
    const char hex_digits[] = "0123456789ABCDEF";

    str[0] = hex_digits[(word >> 12) & 0x0F]; 
    str[1] = hex_digits[(word >> 8) & 0x0F];  

    str[2] = hex_digits[(word >> 4) & 0x0F]; 
    str[3] = hex_digits[word & 0x0F];       

    str[4] = '\0'; 

    return str; 
}

inline void memwrite8(uint8_t __xdata *map_address, uint8_t d)
{
    IOM = 0;
    *map_address = d;
}

inline uint8_t memread8(uint8_t __xdata *map_address)
{
    uint8_t d;
    IOM = 0;
    d = *map_address;
    IOM = 0;
    return d;
}

inline void writeRam(uint16_t address, uint8_t d)
{
    memwrite8((__xdata uint8_t *)address, d);
    return;
}

inline uint8_t readRam(uint16_t address)
{
    uint8_t result = memread8((__xdata uint8_t *)address);
    return result;
}

uint8_t promptType(uint8_t *type)
{
    if (!type)
        return 1;

    while (1)
    {
        setTextSize(2);
        fillScreen();
        setCursor(0, 0);
        LCD_string_write("A: Byte\nB:Word\nC:Double Word\n");
        setCursor(0, 175);
        LCD_string_write((const uint8_t __code *)return_msg);

        *type = ascconvert[keyDetect()];
        switch (*type)
        {
        case 'A':
            *type = 1;
            return 1;
        case 'B':
            *type = 2;
            return 1;
        case 'C':
            *type = 4;
            return 1;
        case '1':
            return 0;
        }

        fillScreen();
        setCursor(0, 0);
        LCD_string_write((const uint8_t __code *)err_type_msg);
        LCD_string_write((const uint8_t __code *)any_key_msg);
        while (!keyDetect())
            ;
    }
}

void promptSize(uint16_t *size, uint8_t type)
{
    if (!size)
        return;

    while (1)
    {
        setTextSize(2);
        fillScreen();
        setCursor(0, 0);
        LCD_string_write("Block Size(word): \n0x");
        *size = inAddress();

        if ((uint32_t)(*size) * type >= 0xFFFF)
        {
            fillScreen();
            setCursor(0, 0);
            LCD_string_write("Block Size is to large\nfor the given type.\n");
            LCD_string_write((const uint8_t __code *)any_key_msg);
            while (!keyDetect())
                ;
            continue;
        }
        else if (*size != 0x0000)
        {
            break;
        }

        fillScreen();
        setCursor(0, 0);
        LCD_string_write((const uint8_t __code *)err_size_msg);
        LCD_string_write((const uint8_t __code *)any_key_msg);
        while (!keyDetect())
            ;
        continue;
    }
}

inline void promptStartAddr(uint16_t *startAddr)
{
    if (!startAddr)
        return;
    setTextSize(2);
    fillScreen();
    setCursor(0, 0);
    LCD_string_write("Start\n");
    LCD_string_write((const uint8_t __code *)addr_msg);
    *startAddr = inAddress();
}

inline void promptEndAddr(uint16_t *endAddr)
{
    if (!endAddr)
        return;
    fillScreen();
    setCursor(0, 0);
    LCD_string_write("Destination\n");
    LCD_string_write((const uint8_t __code *)addr_msg);
    *endAddr = inAddress();
}

inline void promptValue(uint8_t *value)
{
    if (!value)
        return;
    setTextSize(2);
    fillScreen();
    setCursor(0, 0);
    LCD_string_write("Value(byte): \n0x");
    *value = inByte();
}

uint8_t getInputs(uint8_t *type, uint16_t *size, uint16_t *startAddr, uint16_t *endAddr, uint8_t *value)
{
    if (!promptType(type))
        return 0;
    promptSize(size, *type);
    promptStartAddr(startAddr);
    promptEndAddr(endAddr);
    promptValue(value);
    return 1;
}

void ramCheck()
{
    uint8_t temp;
    uint8_t temp_byte;
    getInputs(0, 0, 0, 0, &temp);
    fillScreen();
    setCursor(0, 0);
    LCD_string_write("Checking RAM\n");

    for (uint16_t i = 0x0000;; i++)
    {
        writeRam(i, temp);
        temp_byte = readRam(i);
        if (temp_byte != temp)
        {
            LCD_string_write("Error @0x");
            uint8_t *temp_addr = word_to_hexstr(i);
            LCD_string_write(temp_addr);
            return;
        }
        else if ((i % 4096) == 0)
        {
            writeChar('.'); // Show progress every 4KB
        }
        if (i == 0xFFFF)
            break;
    }
    fillScreen();
    setCursor(0, 0);
    LCD_string_write("Verifying 1s Comp\n");
    temp = ~temp;
    for (uint16_t i = 0x0000;; i++)
    {
        writeRam(i, temp);
        temp_byte = readRam(i);
        if (temp_byte != temp)
        {
            LCD_string_write("Error @0x");
            uint8_t *temp_addr = word_to_hexstr(i);
            LCD_string_write(temp_addr);
            return;
        }
        else if ((i % 4096) == 0)
        {
            writeChar('.'); // Show progress every 4KB
        }
        if (i == 0xFFFF)
            break;
    }
    LCD_string_write("\nDone!");
    setCursor(0, 175);
    LCD_string_write((const uint8_t __code *)return_msg);
    while (!keyDetect())
        ;
    return;
}

void ramDump()
{
    uint16_t start_addr;
    uint8_t type;
    uint16_t block_size;

    getInputs(&type, &block_size, &start_addr, 0, 0);

    if ((uint32_t)start_addr + (uint32_t)(type * block_size) > 0x10000)
    {
        fillScreen();
        setCursor(0, 0);
        LCD_string_write("Error : Overflow\n");
        LCD_string_write((const uint8_t __code *)any_key_msg);
        while (!keyDetect())
            ;
        return;
    }

    uint8_t page = 0;
    uint8_t items_per_page = 11;

    while (1)
    {
        fillScreen();
        setCursor(0, 0);
        LCD_string_write("Page : ");
        LCD_string_write(byte_to_hexstr(page));
        LCD_string_write("\n");

        uint16_t start_index = page * items_per_page;
        for (uint8_t i = 0; i < items_per_page && (start_index + i) < block_size; i++)
        {
            uint16_t addr = start_addr + (start_index + i) * type;

            for (uint8_t j = 0; j < type; j++)
            {
                LCD_string_write(byte_to_hexstr(readRam(addr + j)));
                writeChar(' ');
            }
            LCD_string_write("\n");
        }

        LCD_string_write("\n[1]Prev [5]Exit [0]Next");
        uint8_t k = keyDetect();
        if (k == 0 && (page + 1) * items_per_page < block_size)
            page++;
        else if (k == 1 && page > 0)
            page--;
        else if (k == 5)
            break;
    }
}

void ramMove()
{
    uint8_t type;
    uint16_t size;
    uint16_t src;
    uint16_t dst;

    if (getInputs(&type, &size, &src, &dst, 0) == 0)
        return;

    if ((uint32_t)src + (uint32_t)(type * size) > 0x10000 ||
        (uint32_t)dst + (uint32_t)(type * size) > 0x10000)
    {
        fillScreen();
        setCursor(0, 0);
        LCD_string_write("Error : Overflow\n");
        LCD_string_write((const uint8_t __code *)any_key_msg);
        while (!keyDetect())
            ;
        return;
    }

    for (uint16_t i = 0; i < size; i++)
    {
        for (uint8_t j = 0; j < type; j++)
        {
            uint8_t data = readRam(src + i * type + j);
            writeRam(dst + i * type + j, data);
        }
    }

    setCursor(0, 0);
    fillScreen();
    LCD_string_write("\nDone!\n");
    LCD_string_write((const uint8_t __code *)return_msg);
    while (!keyDetect())
        ;
    return;
}

void ramEdit()
{
    fillScreen();
    setTextSize(2);
    setCursor(0, 0);
    LCD_string_write("Address(word): \n0x");
    uint16_t temp_address = inAddress();

    while (1)
    {
        fillScreen();
        setCursor(0, 0);
        LCD_string_write("0x");
        LCD_string_write(word_to_hexstr(temp_address));
        LCD_string_write(" : \n0x");

        uint8_t temp_data = readRam(temp_address);
        LCD_string_write(byte_to_hexstr(temp_data));
        LCD_string_write("\n->0x");

        temp_data = inByte();
        writeRam(temp_address, temp_data);

        setCursor(0, 175);
        LCD_string_write((const uint8_t __code *)return_msg);
        LCD_string_write((const uint8_t __code *)cont_msg);

        while (1)
        {
            uint8_t k = keyDetect();
            if (k != 0x10)
            {
                if (k == 1)
                    break; 
                else if (k == 0)
                    break; 
            }
        }

        if (keyDetect() == 1)
            return; 

        temp_address += 1;
    }
}

void ramFind()
{
    uint16_t size;
    uint8_t value;
    uint16_t startAddr;

    if (getInputs(0, &size, &startAddr, 0, &value) == 0)
        return;

    if ((uint32_t)startAddr + size - 1 > 0xFFFF)
    {
        fillScreen();
        setCursor(0, 0);
        LCD_string_write("Error : Overflow\n");
        LCD_string_write((const uint8_t __code *)any_key_msg);
        while (!keyDetect())
            ;
    }

    count = 0;
    for (uint16_t i = 0; i < size; i++)
    {
        uint16_t addr = startAddr + i;
        if (readRam(addr) == value && count < MAX_SIZE)
        {
            buffer[count++] = addr;
        }
    }

    uint8_t pages = (count + NUM_PER_PAGE - 1) / NUM_PER_PAGE;
    if (pages == 0)
        pages = 1;

    uint8_t page = 0;
    while (1)
    {
        fillScreen();
        setTextSize(2);
        setCursor(0, 0);

        LCD_string_write("Value: 0x");
        LCD_string_write(byte_to_hexstr(value));
        LCD_string_write("\n");

        for (uint8_t pageOffset = 0; pageOffset < NUM_PER_PAGE; pageOffset++)
        {
            uint16_t match_index = page * NUM_PER_PAGE + pageOffset;
            if (match_index >= count)
                break;

            LCD_string_write(word_to_hexstr(buffer[match_index]));
            LCD_string_write(" ");

            if ((pageOffset & 1) == 1)
                LCD_string_write("\n");
        }

        LCD_string_write("\n[1]Prev [5]Exit [0]Next");
        uint8_t k = keyDetect();
        if (k == 0 && (page + 1) < pages)
            page++;
        else if (k == 1 && page > 0)
            page--;
        else if (k == 5)
            return;
    }
}

void ramCount()
{
    uint16_t size;
    uint8_t value;
    uint16_t startAddr;

    if (getInputs(0, &size, &startAddr, 0, &value) == 0)
        return;

    if ((uint32_t)startAddr + size - 1 > 0xFFFF)
    {
        fillScreen();
        setCursor(0, 0);
        LCD_string_write("Error : Overflow\n");
        LCD_string_write((const uint8_t __code *)any_key_msg);
        while (!keyDetect())
            ;
    }

    count = 0;
    for (uint16_t i = 0; i < size; i++)
    {
        uint16_t addr = startAddr + i;
        if (readRam(addr) == value && count < MAX_SIZE)
        {
            buffer[count++] = addr;
        }
    }

    uint8_t pages = (count + NUM_PER_PAGE - 1) / NUM_PER_PAGE;
    if (pages == 0)
        pages = 1;

    uint8_t page = 0;
    while (1)
    {
        fillScreen();
        setTextSize(2);
        setCursor(0, 0);

        LCD_string_write("Count: 0x");
        LCD_string_write(word_to_hexstr(count));
        LCD_string_write("\n");
        LCD_string_write("Value: 0x");
        LCD_string_write(byte_to_hexstr(value));
        LCD_string_write("\n");

        for (uint8_t pageOffset = 0; pageOffset < NUM_PER_PAGE; pageOffset++)
        {
            uint16_t match_index = page * NUM_PER_PAGE + pageOffset;
            if (match_index >= count)
                break;

            LCD_string_write(word_to_hexstr(buffer[match_index]));
            LCD_string_write(" ");

            if ((pageOffset & 1) == 1)
                LCD_string_write("\n");
        }

        LCD_string_write("\n[1]Prev [5]Exit [0]Next");
        uint8_t k = keyDetect();
        if (k == 0 && (page + 1) < pages)
            page++;
        else if (k == 1 && page > 0)
            page--;
        else if (k == 5)
            return;
    }
}