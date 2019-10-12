/*
 * routingtable.c
 *
 * Created: 10/09/2019 12:28:02
 *  Author: Mike
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "routingtable.h"
#include "messages.h"

#define		MAXNODES	255

uint8_t **aRoutingTable	= NULL;
uint8_t *aNeighbors		= NULL;
uint8_t uNeighbors		= 0;
uint8_t *aMissedBroadcasts = NULL;
uint8_t uKnownNodes		= 0;
uint8_t uMyID			= 0;

uint8_t *aRoutingString = NULL;

void init_RoutingTable(uint8_t _myid)
{
	aRoutingTable			= (uint8_t**) calloc(MAXNODES + 1, sizeof(uint8_t*));
	aRoutingTable[_myid]	= (uint8_t *) calloc(MAXNODES + 1, sizeof(uint8_t));
	aNeighbors				= (uint8_t *) calloc(MAXNODES, sizeof(uint8_t));
	aMissedBroadcasts		= (uint8_t *) calloc(MAXNODES + 1, sizeof(uint8_t));
	aRoutingString			= (uint8_t *) calloc(255, sizeof(uint8_t));
	
	uMyID = _myid;
}

void addKnownNode(uint8_t uNodeID)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON);
	if ( (!isKnown(uNodeID)) && (uNodeID != uMyID) )
		{
			aRoutingTable[uNodeID] = (uint8_t*) calloc(MAXNODES + 1, sizeof(uint8_t));
			uKnownNodes++;
		}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE);
}

uint8_t isKnown(uint8_t uNodeID)
{
	return aRoutingTable[uNodeID] != NULL;
}

void addNeighbor(uint8_t uNodeID)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON);
	if (memchr(aNeighbors, uNodeID, MAXNODES) == NULL)
		{
			addKnownNode(uNodeID);
			aNeighbors[uNeighbors] = uNodeID;
			uNeighbors++;
		}
		
	aMissedBroadcasts[uNodeID] = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE);
}

void removeNeighbor(uint8_t uNodeID)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON);
	uint8_t *pNode = memchr(aNeighbors, uNodeID, MAXNODES);
	uint8_t position = pNode - aNeighbors;
	
	if (pNode != NULL)
	{
		for (uint8_t i = 0; i < (MAXNODES - position); i++)
		{
			*(pNode + i) = *(pNode + i + 1);
		}
		
		if (uNeighbors == MAXNODES)
		{
			*(aNeighbors + MAXNODES) = 0;
		}
		uNeighbors--;
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE);
}

void updateNeighborList(void)
{
	for (uint8_t i = 0; i < uNeighbors; i++)
	{
		aMissedBroadcasts[aNeighbors[i]]++;
		
		if (aMissedBroadcasts[aNeighbors[i]] >= 3)
		{
			removeNeighbor(aNeighbors[i]);
		}
	}
}

void FillRoutingTable(uint8_t *routingstring, uint8_t string_length)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON);
	memset( aRoutingTable[ routingstring[1] ], 0, MAXNODES + 1 );
	
	if(string_length > 3)
	{
		if (string_length > 32) 
		{
			string_length = 32;
		}
		
		for(uint8_t i = 3; i < string_length; i += 2 ) {
		
			if ( (routingstring[i] != uMyID) && (routingstring[i] != 0) && !isNeighbor( routingstring[i] ) )
			{
				addKnownNode( routingstring[i] );
				if (routingstring[ i + 1] <= uKnownNodes)
				{
					aRoutingTable[ routingstring[1] ][ routingstring[i] ] = routingstring[ i + 1 ] + 1;
				}
			}
		}
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE);
}

uint8_t isNeighbor(uint8_t uNodeID)
{
	return memchr(aNeighbors, uNodeID, MAXNODES) != NULL;
}

tNeighborHops findLeastHops(uint8_t uNodeID)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON);
	tNeighborHops NnH = {0 , UINT8_MAX};
		
	if (isKnown(uNodeID))
	{
		if (isNeighbor(uNodeID))
		{
			NnH.uHops = 1;
			NnH.uNeighbor = uNodeID;
			return NnH;
		}

		for (uint8_t i = 0; i < uNeighbors; i++)
		{
			if ((aRoutingTable[ aNeighbors[i] ][uNodeID] < NnH.uHops) && (aRoutingTable[ aNeighbors[i] ][uNodeID] > 0))
			{
				NnH.uHops = aRoutingTable[ aNeighbors[i] ][uNodeID];
				NnH.uNeighbor = aNeighbors[i];
			}
		}
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE);
	return NnH;
}

uint8_t* getRoutingString( void )
{
	ATOMIC_BLOCK(ATOMIC_FORCEON);
	tNeighborHops NnH;
	uint8_t Idx = 2;
	
	memset(aRoutingString, 0, 255);
	
	for (uint8_t ID = 0; ID < 255; ID++)
	{
		if ( isKnown(ID) )
		{
			NnH = findLeastHops(ID);
			
			if (NnH.uNeighbor != 0)
			{
				aRoutingString[++Idx] = ID;
				aRoutingString[++Idx] = NnH.uHops;
			}
		}
	}
	
	aRoutingString[0] = ROUTINGHEADER;
	aRoutingString[1] = uMyID;
	aRoutingString[2] = Idx + 1;
	
	printf("Generated Routingstring: ");
	printf_Routing(aRoutingString, aRoutingString[2]);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE);
	return aRoutingString;
}