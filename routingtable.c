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

uint8_t uRoutingTailLen = 0;
uint8_t uExtraRoutingPackets = 0;
uint8_t aRoutingPackets[8][32];


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
	uint8_t originId = routingstring[1];
	
	for(uint8_t i = 2; i < string_length; i += 2 ) 
	{
		uint8_t stringNodeId = routingstring[i];
		uint8_t hopCnt = routingstring[i + 1];
		
		if ( (stringNodeId != uMyID) && (stringNodeId != 0) && !isNeighbor( stringNodeId ) )
		{
			addKnownNode( stringNodeId );
			writeHopCount( originId, stringNodeId, hopCnt + 1);
			resetAge( originId, stringNodeId );
		}
	}
}

uint8_t isNeighbor(uint8_t uNodeID)
{
	return memchr(aNeighbors, uNodeID, MAXNODES) != NULL;
}

tNeighborHops findFewestHops(uint8_t uNodeID)
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
			NnH = findFewestHops(ID);
			
			if (( NnH.uHops != 255 ) && ( NnH.uNeighbor != 0 ))
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

void chopRoutingString(){
	uint8_t _tmpPacket[32];
	uint8_t _pos = 0;
	
	_tmpPacket[0] = ROUTINGHEADER;
	_tmpPacket[1] = uMyID;
	
	uRoutingTailLen = uRoutingStringLength % 32;
	uExtraRoutingPackets = (uRoutingStringLength - uRoutingTailLen) / 32;
	
	memset(aRoutingPackets, 0, sizeof aRoutingPackets);
	
	memcpy(aRoutingPackets[0], aRoutingString, 32);
	
	for(uint8_t _pckt = 1; _pckt <= uExtraRoutingPackets; _pckt++ ){
		_pos += 32;
		memset(&_tmpPacket[2], 0, 30);
		if(_pckt < uExtraRoutingPackets) memcpy(&_tmpPacket[2], &aRoutingString[_pos], 30);
		else memcpy(&_tmpPacket[2], &aRoutingString[_pos], uRoutingTailLen);
		memcpy(aRoutingPackets[_pckt], _tmpPacket, 32);
	}
}

uint8_t readHopCount(uint8_t uRow, uint8_t uNode)
{
	return aRoutingTable[uRow][uNode] & 0x3F;
}

uint8_t readAge(uint8_t uRow, uint8_t uNode)
{
	return (aRoutingTable[uRow][uNode] & 0xC0) >> 6;
}

void writeHopCount(uint8_t uRow, uint8_t uNode, uint8_t hopCount) {
	if (hopCount > 63)	//CHeck if the new hopCount isn't larger than a 6 bit number
	{
		hopCount = 63;
	}
	
	aRoutingTable[uRow][uNode] &= 0xC0;
	aRoutingTable[uRow][uNode] |= hopCount;
}

void incrementAge(uint8_t uRow, uint8_t uNode)
{
	if ( (aRoutingTable[uRow][uNode] & 0xC0) != 0xC0 )
	{
		aRoutingTable[uRow][uNode] += 0x40;
	}
}

void resetAge(uint8_t uRow, uint8_t uNode)
{
	aRoutingTable[uRow][uNode] &= 0xC0;
}