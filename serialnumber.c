/*
 * serialnumber.c
 *
 * Created: 27-9-2019 14:27:48
 *  Author: felix
 */ 
#include <stdlib.h>
#include <avr/io.h>
#include <string.h>

#include "serialnumber.h"

typedef struct serialidpair {
	uint8_t serial[11];
	uint8_t id;
} SerialIdPair;

const SerialIdPair SerialIdLookup[] =
{
	{ {53,88,48,56,53,56,19,20,0,20,0 } , 51},	// Felix
	{ {53,88,48,56,53,56,17,12,0,0,0 } , 52},	// Rowan
	{ {0,0,0,0,0,0,0,0,0,0,0 } , 53},	// Stephan
	{ {53,88,48,56,49,49,13,22,0,7,0 } , 77},	// Jannick
	{ {53,88,48,56,53,56,6,6,0,21,0 } , 83},	// Mike
	{ {53,88,48,56,49,49,13,5,0,8,0 } , 42}		// One of the Kit's xmega

};

	
uint8_t GetIdFromLookup(uint8_t* serial)
{
	for (int i = 0; i < sizeof SerialIdLookup / sizeof SerialIdLookup[0]; i++)
	{
		if(memcmp(SerialIdLookup[i].serial, serial, 11) == 0)
		{
			return SerialIdLookup[i].id;
		}
	}
	return 0;		// Not found
}

uint8_t ReadSignatureByte(uint16_t Address) {
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	uint8_t Result;
	asm ("lpm %0, Z\n" : "=r" (Result) : "z" (Address));
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return Result;
}

void NVM_GetDeviceSerial(uint8_t* b) {
	enum {
		LOTNUM0=8,  // Lot Number Byte 0, ASCII
		LOTNUM1,    // Lot Number Byte 1, ASCII
		LOTNUM2,    // Lot Number Byte 2, ASCII
		LOTNUM3,    // Lot Number Byte 3, ASCII
		LOTNUM4,    // Lot Number Byte 4, ASCII
		LOTNUM5,    // Lot Number Byte 5, ASCII
		WAFNUM =16, // Wafer Number
		COORDX0=18, // Wafer Coordinate X Byte 0
		COORDX1,    // Wafer Coordinate X Byte 1
		COORDY0,    // Wafer Coordinate Y Byte 0
		COORDY1,    // Wafer Coordinate Y Byte 1
	};

	b[0]=ReadSignatureByte(LOTNUM0);
	b[1]=ReadSignatureByte(LOTNUM1);
	b[2]=ReadSignatureByte(LOTNUM2);
	b[3]=ReadSignatureByte(LOTNUM3);
	b[4]=ReadSignatureByte(LOTNUM4);
	b[5]=ReadSignatureByte(LOTNUM5);
	b[6]=ReadSignatureByte(WAFNUM);
	b[7]=ReadSignatureByte(COORDX0);
	b[8]=ReadSignatureByte(COORDX1);
	b[9]=ReadSignatureByte(COORDY0);
	b[10]=ReadSignatureByte(COORDY1);
}