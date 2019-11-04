/*
 * routingtable.h
 *
 * Created: 10/09/2019 12:27:47
 *  Author: Mike
 */ 


#ifndef ROUTINGTABLE_H_
#define ROUTINGTABLE_H_

typedef struct{
	uint8_t uNeighbor;
	uint8_t uHops;
	} tNeighborHops;

uint8_t uRoutingStringLength;
void init_RoutingTable(uint8_t uMyID);
void addKnownNode(uint8_t uNodeID );
uint8_t isKnown(uint8_t uNodeID);
void addNeighbor(uint8_t uNodeID);
void FillRoutingTable(uint8_t *routingstring, uint8_t string_length);
void removeNeighbor(uint8_t uNodeID);
void updateNeighborList(void);
uint8_t isNeighbor(uint8_t uNodeID);
tNeighborHops findFewestHops(uint8_t uNodeID);
uint8_t* getRoutingString( void );
void chopRoutingString( void );
uint8_t uExtraRoutingPackets;
uint8_t aRoutingPackets[8][32];
uint8_t	uRoutingTailLen;
uint8_t uTotRoutingPackets;
uint8_t readHopCount(uint8_t uRow, uint8_t uNode);
uint8_t readAge(uint8_t uRow, uint8_t uNode);
void writeHopCount(uint8_t uRow, uint8_t uNode, uint8_t hopCount);
void incrementAge(uint8_t uRow, uint8_t uNode);
void resetAge(uint8_t uRow, uint8_t uNode);
#endif /* ROUTINGTABLE_H_ */