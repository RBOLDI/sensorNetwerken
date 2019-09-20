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


	
tTableElement *aRoutingTable = NULL;
uint8_t *aExtantNodes = NULL;
uint8_t uExtantNodes = 0;

uint8_t *aRoutingString = NULL;


#if (MAXNEIGHBORS > UINT8_MAX)
#error "MAXNEIGHBORS won't fit in tNodeID. Change type of tNodeID."
#endif

void init_routingtable( void )
{
	aRoutingTable = (tTableElement*) calloc(MAXNODES + 1, sizeof(tTableElement));
	aExtantNodes = (uint8_t *) calloc(MAXNODES, sizeof(uint8_t));
	aRoutingString = (uint8_t*) calloc((MAXNODES * 2) + 3, sizeof(uint8_t));
	
	//For testing
	for (uint8_t i = 0; i < 20; i++)
	{
		addneighbor(i+48);
	}
}

void addneighbor(uint8_t NodeID)
{
	aRoutingTable[NodeID].uHops = 1;
	aRoutingTable[NodeID].NodeID = NodeID;
	
	if (strchr((char*) aExtantNodes, NodeID) == NULL)
	{
		aExtantNodes[strlen((char*) aExtantNodes)] = NodeID;
		uExtantNodes++;
	}
}

void removeneighbor(uint8_t NodeID)
{
	aRoutingTable[NodeID].uHops = 0;
	aRoutingTable[NodeID].NodeID = 0;
}

uint8_t sendtowho(uint8_t TargetID)
{
	return aRoutingTable[TargetID].NodeID;
}

uint8_t* GetRoutingString(uint8_t myID)
{
	//Clear string
	memset(aRoutingString, 0, (MAXNODES * 2 + 3) * sizeof(uint8_t));
	
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