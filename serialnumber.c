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
	{ {53,88,48,56,53,56,23,12,0,0,0 } , 53},	// Stephan
	{ {53,88,48,56,49,49,13,22,0,7,0 } , 77},	// Jannick
	{ {53,88,48,56,53,56,6,6,0,21,0 } , 83},	// Mike
	{ {53,88,48,56,49,49,13,5,0,8,0 } , 105},	// Base station xmega
	{ {53,88,48,56,49,49,14,15,0,10,0 } , 43}	// Anchor station xmega (not used)
};

/*
	The GetIdFromLookup function is to retrieve the student dedicated ID by reading 
	the internal unique serial number each Xmega has stored in its signature memory.
	If the argument serial matches one in the lookup table SerialIdLookup, it will
	return the paired ID. This replaces ID dedication by the use of selector pins.
*/
uint8_t GetIdFromLookup(uint8_t* serial)
{
	for (uint8_t i = 0; i < sizeof SerialIdLookup / sizeof SerialIdLookup[0]; i++)
	{
		if(memcmp(SerialIdLookup[i].serial, serial, 11) == 0)
		{
			return SerialIdLookup[i].id;
		}
	}
	return 0;		// Not found
}

/*
	The ReadSignatureByte reads a byte of memory at the address that was given as
	argument. We use it to retrieve the chips serial number.
*/
uint8_t ReadSignatureByte(uint16_t Address) {
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	uint8_t Result;
	asm ("lpm %0, Z\n" : "=r" (Result) : "z" (Address));
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return Result;
}

/*
	The NVM_GetDeviceSerial function changes the value that was given as argument to
	the devices serial number, spread over 11 bytes of memory.
*/
void NVM_GetDeviceSerial(uint8_t* b) {
	b[0]=ReadSignatureByte(8);		// Lot Number Byte 0, ASCII
	b[1]=ReadSignatureByte(9);		// Lot Number Byte 1, ASCII
	b[2]=ReadSignatureByte(10);		// Lot Number Byte 2, ASCII
	b[3]=ReadSignatureByte(11);		// Lot Number Byte 3, ASCII
	b[4]=ReadSignatureByte(12);		// Lot Number Byte 4, ASCII
	b[5]=ReadSignatureByte(13);		// Lot Number Byte 5, ASCII
	b[6]=ReadSignatureByte(16);		// Wafer Number
	b[7]=ReadSignatureByte(18);		// Wafer Coordinate X Byte 0
	b[8]=ReadSignatureByte(19);		// Wafer Coordinate X Byte 1
	b[9]=ReadSignatureByte(20);		// Wafer Coordinate Y Byte 0
	b[10]=ReadSignatureByte(21);	// Wafer Coordinate Y Byte 1
}