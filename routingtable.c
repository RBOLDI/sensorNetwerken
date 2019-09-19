/*
 * routingtable.c
 *
 * Created: 10/09/2019 12:28:02
 *  Author: Mike
 */ 

#include <avr/io.h>
#include <string.h>
#include <stdlib.h>

#include "routingtable.h"

#define		MAXNODES	255

#define		BROADCAST	0x01
#define		RRTABLE		0x02

/*
The routingtable is an array of MAXNEIGHBORS structs. 
The Nth member of the array holds the data for the SensorNode with ID N.
The first number in the struct is the number of hops necessary to reach the target Node, 
the second number is the ID of a connected SensorNode that is closer to the destination.

There is a second array called ConectedNodes of all the IDs with which there is a direct connection.
*/


typedef struct
{																
	uint8_t uHops;
	tNodeID NodeID;
} tTableElement;
	
tTableElement *aRoutingTable = NULL;
tNodeID *aExtantNodes = NULL;
uint8_t uExtantNodes;

uint8_t *aRoutingString = NULL;


#if (MAXNEIGHBORS > UINT8_MAX)
#error "MAXNEIGHBORS won't fit in tNodeID. Change type of tNodeID."
#endif

void init_routingtable( void )
{
	aRoutingTable = (tTableElement*) calloc(MAXNODES, sizeof(tTableElement));
	aExtantNodes = (tNodeID *) calloc(MAXNODES, sizeof(uint8_t));
	aRoutingString = (uint8_t*) calloc((MAXNODES * 2) + 2, sizeof(uint8_t));
	
	//For testing
	for (uint8_t i = 1; i < 66; i++)
	{
		addneighbor((rand() % 100) + 1);
	}
}

void addneighbor(tNodeID NodeID)
{
	aRoutingTable[NodeID - 1].uHops = 1;
	aRoutingTable[NodeID - 1].NodeID = NodeID;
	
	if (strchr((char*) aExtantNodes, NodeID) == NULL)
	{
		aExtantNodes[strlen((char*) aExtantNodes)] = NodeID;
		uExtantNodes++;
	}
}

void removeneighbor(tNodeID NodeID)
{
	aRoutingTable[NodeID - 1].uHops = 0;
	aRoutingTable[NodeID - 1].NodeID = 0;
}

tNodeID sendtowho(tNodeID TargetID)
{
	return aRoutingTable[TargetID - 1].NodeID;
}

uint8_t* GetRoutingString(uint8_t myID)
{
	//Clear string
	memset(aRoutingString, 0, sizeof(aRoutingString)/sizeof(uint8_t));
	
	//RoutingString := Berichttype EigenID NodeID Hopcnt
	uint8_t j = 2;
	
	//Walk through aExtantNodes, filling string with NodeID and Hopcnt on the way
	for(uint8_t i = 0; i < uExtantNodes; i++)
	{
		if(aRoutingTable[aExtantNodes[i]].uHops != 0)
		{
			aRoutingString[++j] = aRoutingTable[aExtantNodes[i]].NodeID;
			aRoutingString[++j] = aRoutingTable[aExtantNodes[i]].uHops;
		}
	}
	
	//String prefix
	aRoutingString[0] = RRTABLE;
	aRoutingString[1] = myID;
	aRoutingString[2] = j+1;
	
	return aRoutingString;
}