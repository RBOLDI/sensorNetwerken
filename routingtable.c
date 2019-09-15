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

#define  MAXNEIGHBORS		255

/*The routingtable is an array of MAXNEIGHBORS structs. 
The Nth member of the array holds the data for the SensorNode with ID N.
The first number in the struct is the number of hops necessary to reach the target Node, 
the second number is the ID of a connected SensorNode that is closer to the destination
*/


typedef struct
{																
	uint8_t uHops;
	tNodeID NeighborID;
} tTableElement;
	
tTableElement* aRoutingTable = NULL;

tNodeID* aConnectedNodes = NULL;

void init_routingtable( void )
{
	aRoutingTable = (tTableElement*) calloc(MAXNEIGHBORS, sizeof(tTableElement));
	aConnectedNodes = (tNodeID*) calloc(MAXNEIGHBORS, sizeof(tNodeID));
}

void addneighbor(tNodeID NodeID)
{
	if (strchr(aConnectedNodes, NodeID) == NULL)
	{
		aConnectedNodes[strlen(aConnectedNodes)] = NodeID;
		(aRoutingTable + NodeID -1)->uHops = 1;
		(aRoutingTable + NodeID -1)->NeighborID = NodeID;
	}
}

void removeneighbor(tNodeID NodeID)
{
	tNodeID *pIndex = (tNodeID *) strchr(aConnectedNodes, NodeID);
	size_t Size = strlen(aConnectedNodes);
	
	#if (MAXNEIGHBORS > SIZE_MAX)
	#error "MAXNEIGHBORS won't fit in size_t. Change type of uSize."
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
			*(aConnectedNodes + MAXNEIGHBORS -1) = 0;
		}
	}
}