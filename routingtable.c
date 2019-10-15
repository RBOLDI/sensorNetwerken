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

#include "routingtable.h"
#include "messages.h"
#include "debug_opts.h"

#define		MAXNODES	255

uint8_t **aRoutingTable	= NULL;
uint8_t *aNeighbors		= NULL;
uint8_t uNeighbors		= 0;
uint8_t *aMissedBroadcasts = NULL;
uint8_t uKnownNodes		= 0;
uint8_t uMyID			= 0;

uint8_t *aRoutingString = NULL;
uint8_t uRoutingStringLength = 0;


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
	if ( (!isKnown(uNodeID)) && (uNodeID != uMyID) )
	{
		aRoutingTable[uNodeID] = (uint8_t*) calloc(MAXNODES + 1, sizeof(uint8_t));
		uKnownNodes++;
	}
}

uint8_t isKnown(uint8_t uNodeID)
{
	return aRoutingTable[uNodeID] != NULL;
}

void addNeighbor(uint8_t uNodeID)
{
	if (memchr(aNeighbors, uNodeID, MAXNODES) == NULL)
		{
			addKnownNode(uNodeID);
			aNeighbors[uNeighbors] = uNodeID;
			uNeighbors++;
			printf_SetColor(COLOR_CYAN);
			DB_MSG(("New Neighbor: %d\r\n", uNodeID));
			printf_SetColor(COLOR_RESET);
		}
		
	aMissedBroadcasts[uNodeID] = 0;
}

void removeNeighbor(uint8_t uNodeID)
{
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

		printf_SetColor(COLOR_MAGENTA);
		DB_MSG(("Lost Neighbor: %d\r\n", uNodeID));
		printf_SetColor(COLOR_RESET);
	}
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
	memset( aRoutingTable[ routingstring[1] ], 0, MAXNODES + 1 );
	
	if(string_length > 2)
	{
		if (string_length > 32) 
		{
			string_length = 32;
		}
		
		for(uint8_t i = 2; i < string_length; i += 2 ) 
		{
			if ( (routingstring[i] != uMyID) && (routingstring[i] != 0) && !isNeighbor( routingstring[i] ) )
			{
				addKnownNode( routingstring[i] );
				aRoutingTable[ routingstring[1] ][ routingstring[i] ] = routingstring[ i + 1 ] + 1;
			}
		}
	}
}

uint8_t isNeighbor(uint8_t uNodeID)
{
	return memchr(aNeighbors, uNodeID, MAXNODES) != NULL;
}

tNeighborHops findLeastHops(uint8_t uNodeID)
{
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
			if ((aRoutingTable[ aNeighbors[i] ][uNodeID] < NnH.uHops) && (aRoutingTable[ aNeighbors[i] ][uNodeID] > 0) && (aRoutingTable[ aNeighbors[i] ][uNodeID] < uKnownNodes) )
			{
				NnH.uHops = aRoutingTable[ aNeighbors[i] ][uNodeID];
				NnH.uNeighbor = aNeighbors[i];
			}
		}
	}
	return NnH;
}

uint8_t* getRoutingString( void )
{
	tNeighborHops NnH;
	uint8_t Idx = 1;
	
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
	uRoutingStringLength = Idx + 1;
	
	printf_SetColor(COLOR_YELLOW);
	printf_Routing(aRoutingString, uRoutingStringLength);
	printf_SetColor(COLOR_RESET);
	
	return aRoutingString;
}