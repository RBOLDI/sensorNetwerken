/*
 * routingtable.c
 *
 * Created: 10/09/2019 12:28:02
 *  Author: Mike
 */ 

#include <avr/io.h>
#include <string.h>
#include "routingtable.h"


uint8_t aRoutingTable[256][32];													//256 arrays with 256 bits, position (index * 8 + bitposition) corresponds to ID, 
																				//bits used as boolean (is ID neighbor?)
uint8_t uMyID;

void copyrow( uint8_t uIDRow, uint8_t* aRow ){									//Copies neighbor list received from neighbor to table
	if ( sizeof(aRow) == 32 ) memcpy( aRoutingTable[uIDRow], aRow , 32);
}

void addneighbor( uint8_t uIDNeighbor ){											//Function for adding own neighbor to table
	aRoutingTable[uMyID][uIDNeighbor / 8] |= 0x01 << (uIDNeighbor % 8);
}

void removeneighbor( uint8_t uIDNeighbor ){										//Function for removing own neighbor from table
	aRoutingTable[uMyID][uIDNeighbor / 8] &= ~(0x01 << (uIDNeighbor % 8));
}

uint8_t* getownlist( void ){													//Returns a pointer to own neighbor list for sending to neighbor
	return aRoutingTable[uMyID];
}

uint8_t* getotherlist( uint8_t uIDSensorNode ){									//Returns pointer to neighbor list of other node for sending to neighbor
	return aRoutingTable[uIDSensorNode];
}

void init_routingtable( uint8_t uNodeID ){
	uMyID = uNodeID;
	for (uint8_t i = 0; i <= 255; i++) memset(aRoutingTable[i], 0, 32);
}