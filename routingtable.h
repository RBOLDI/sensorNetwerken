/*
 * routingtable.h
 *
 * Created: 10/09/2019 12:27:47
 *  Author: Mike
 */ 


#ifndef ROUTINGTABLE_H_
#define ROUTINGTABLE_H_

typedef struct
{
	uint8_t uHops;
	uint8_t NodeID;
} tTableElement;

void init_routingtable( void );
void addneighbor(uint8_t NodeID);
void removeneighbor(uint8_t NodeID);
uint8_t sendtowho(uint8_t);
uint8_t* GetRoutingString(uint8_t myID);
#endif /* ROUTINGTABLE_H_ */