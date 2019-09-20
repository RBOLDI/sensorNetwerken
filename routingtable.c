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
	for (uint8_t i = 1; i <= 50; i++)
	{
		addneighbor(i);
	}
}

void addneighbor(uint8_t NodeID)
{
	aRoutingTable[NodeID].uHops = 1;
	aRoutingTable[NodeID].NeighborID = NodeID;
	
	if (strchr((char*) aExtantNodes, NodeID) == NULL)
	{
		aExtantNodes[strlen((char*) aExtantNodes)] = NodeID;
		uExtantNodes++;
	}
}

void removeneighbor(uint8_t NodeID)
{
	aRoutingTable[NodeID].uHops = 0;
	aRoutingTable[NodeID].NeighborID = 0;
}

uint8_t sendtowho(uint8_t TargetID)
{
	return aRoutingTable[TargetID].NeighborID;
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
			aRoutingString[++j] = aExtantNodes[i];
			aRoutingString[++j] = aRoutingTable[aExtantNodes[i]].uHops;
		}
	}
	
	//String prefix
	aRoutingString[0] = RRTABLE;
	aRoutingString[1] = myID;
	aRoutingString[2] = j+1;
	
	return aRoutingString;
}

void FillRoutingTable(uint8_t *routingstring, uint8_t string_length)
{
	if(string_length <= 3) return;
	
	memset(aRoutingTable, 0, (MAXNODES + 1) * sizeof(tTableElement) );	// Clear existing values

	for(uint8_t i = 3; i < string_length; i+=2 ) {
		aRoutingTable[ routingstring[i] ].NeighborID = 255; /* Closest neighbor WIP*/
		aRoutingTable[ routingstring[i] ].uHops = routingstring[i+1];
	}
}