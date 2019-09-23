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


uint8_t **aRoutingTable	= NULL;
uint8_t *aNeighbors		= NULL;
uint8_t uNeighbors		= 0;
uint8_t uKnownNodes		= 0;
uint8_t uMyID			= 0;

uint8_t *aRoutingString = NULL;

void init_RoutingTable(uint8_t uMyID)
{
	aRoutingTable			= (uint8_t**) calloc(MAXNODES + 1, sizeof(uint8_t*));
	aRoutingTable[uMyID]	= (uint8_t *) calloc(MAXNODES + 1, sizeof(uint8_t));
	aNeighbors				= (uint8_t *) calloc(MAXNODES, sizeof(uint8_t));
	aRoutingString			= (uint8_t *) calloc(255, sizeof(uint8_t));
}

void addKnownNode(uint8_t uNodeID )
{
	if (aRoutingTable[uNodeID] == NULL)
		{
			aRoutingTable[uNodeID] = (uint8_t*) calloc(MAXNODES + 1, sizeof(uint8_t));
			uKnownNodes++;
		}
}

void isKnown(uint8_t uNodeID)
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
		}
}

void removeNeighbor(uint8_t uNodeID)
{
	uint8_t *pNode = memchr(aNeighbors, uNodeID, MAXNODES);
	uint8_t position = pNode - aNeighbors;
	
	if (pNode != NULL)
	{
		for (uint8_t i = 0; i < (MAXNODES - position); i++)
		{
			*(pNode + i) = *(pNode + i + 1)
		}
		
		if (uNeighbors == MAXNODES)
		{
			*(aNeighbors + MAXNODES) = 0;
		}
		
		uNeighbors--;
	}
}

