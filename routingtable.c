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

#define		MAXNEIGHBORS	255
#define		RXPTABLE		0x03
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
	tNodeID NeighborID;
} tTableElement;
	
tTableElement *aRoutingTable = NULL;

tNodeID *aConnectedNodes = NULL;

#if (MAXNEIGHBORS > UINT8_MAX)
#error "MAXNEIGHBORS won't fit in tNodeID. Change type of tNodeID."
#endif

uint8_t *aRoutingString = NULL;

void init_routingtable( void )
{
	aRoutingTable = (tTableElement*) calloc(MAXNEIGHBORS, sizeof(tTableElement));
	aConnectedNodes = (tNodeID*) calloc(MAXNEIGHBORS, sizeof(tNodeID));
	aRoutingString = (uint8_t*) calloc((MAXNEIGHBORS * 2) + 2, sizeof(uint8_t));
}

void addneighbor(tNodeID NodeID)
{
	if (strchr(aConnectedNodes, NodeID) == NULL)
	{
		aConnectedNodes[strlen(aConnectedNodes)] = NodeID;
		(aRoutingTable + NodeID - 1)->uHops = 1;
		(aRoutingTable + NodeID - 1)->NeighborID = NodeID;
	}
}

void removeneighbor(tNodeID NodeID)
{
	tNodeID *pIndex = (tNodeID *) strchr(aConnectedNodes, NodeID);
	size_t Size = strlen(aConnectedNodes);
	
	#if (MAXNEIGHBORS > SIZE_MAX)
	#error "MAXNEIGHBORS won't fit in size_t. Change type of Size."
	#endif
	
	if (pIndex != NULL)
	{
		while (pIndex < (aConnectedNodes + Size))
		{
			*pIndex = *(pIndex + 1);
			pIndex++;
		}
		
		if (Size == MAXNEIGHBORS)
		{
			*(aConnectedNodes + MAXNEIGHBORS - 1) = 0;
		}
		
		(aRoutingTable + NodeID - 1)->uHops = 0;						
		(aRoutingTable + NodeID - 1)->NeighborID = 0x00;
	}
}

tNodeID sendtowho(tNodeID TargetID)
{
	return (aRoutingTable + TargetID - 1)->NeighborID;
}

uint8_t* GetRoutingString(uint8_t myID)
{
	// := berichttype EigenID NodeID Hopcnt
	// aRoutingTable[0] = ID 1
	uint8_t j = 2;
	
	//String prefix
	aRoutingString[0] = RXPTABLE;
	aRoutingString[1] = myID;
	
	//Loop through table, filling string NodeID and Hopcnt on the way
	for(uint8_t i = 1; i == MAXNEIGHBORS; i++)
	{
		if(aRoutingTable[i].NeighborID != 0 )
		{
			aRoutingString[j] = aRoutingTable[i].NeighborID;
			aRoutingString[j + 1] = aRoutingTable[i].uHops;
			j++;
		}
	}
	//Return String
	return * aRoutingString;
}

char* AppendtoReceivedRoutingString(char* ReceivedRoutingString)
{
	/*TO DO*/
}