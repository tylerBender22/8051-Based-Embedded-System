/*
 * Project Name: 8051 Microcontroller
 * File Name: ramDriver.c
 * Description: Header for Memory Drivers for the 8051 Microcontroller.
 *
 * Author: Tyler Bender
 * Date: May 13, 2025
 *
 */

#ifndef RAMDRIVER_H
#define RAMDRIVER_H

#include "lcdDriver.h"
#include <8051.h>
#include <stdint.h>

#define START_RAM 0x0000
#define END_RAM 0xFFFF

#define MAX_SIZE 16
#define NUM_PER_PAGE 16


void ramCheck();

void ramDump();

void ramMove();

void ramEdit();

void ramFind();

void ramCount();

uint8_t promptType(uint8_t *type);

void promptSize(uint16_t *size, uint8_t type);

void promptStartAddr(uint16_t *startAddr);

void promptEndAddr(uint16_t *endAddr);

void promptValue(uint8_t *value);

uint8_t *byte_to_hexstr(uint8_t byte);

uint8_t *word_to_hexstr(uint16_t word);

uint8_t getInputs(uint8_t *type, uint16_t *size, uint16_t *startAddr, uint16_t *endAddr, uint8_t *value);

extern uint8_t keyDetect();
extern uint8_t inByte();
extern uint16_t inAddress();
extern __code uint8_t ascconvert[];
extern __idata uint16_t count;
extern __idata uint16_t buffer[]; 

extern __code uint8_t return_msg[];
extern __code uint8_t any_key_msg[];
extern __code uint8_t cont_msg[];
extern __code uint8_t addr_msg[];
extern __code uint8_t value_msg[];
extern __code uint8_t err_type_msg[];
extern __code uint8_t err_size_msg[];

#endif