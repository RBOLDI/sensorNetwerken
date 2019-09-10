/*
 * routingtable.c
 *
 * Created: 10/09/2019 12:28:02
 *  Author: Mike
 */ 

#include <avr/io.h>
#include <string.h>
#include "routingtable.h"


uint8_t aRoutingTable[256][32];													//256 arrays with 256 bits, position corresponds to ID, bits used as boolean (is ID neighbor, true false)

void copyrow( uint8_t idrow, uint8_t* row ){									//Copies neighbor list received from neighbor to table
	if ( sizeof(row) == 32 ) memcpy( aRoutingTable[idrow], row , 32);
}

void addneighbor( uint8_t idneighbor ){											//Function for adding own neighbor to table
	aRoutingTable[getID()][idneighbor / 8] |= 0x01 << (idneighbor % 8);
}

void removeneighbor( uint8_t idneighbor ){										//Function for removing own neighbor from table
	aRoutingTable[getID()][idneighbor / 8] &= ~(0x01 << (idneighbor % 8));
}

uint8_t* getownlist( void ){													//Returns a pointer to own neighbor list for sending to neighbor
	return aRoutingTable[getID()];
}

uint8_t* getotherlist( uint8_t idsensornode ){									//Returns pointer to neighbor list of other node for sending to neighbor
	return aRoutingTable[idsensornode];
}