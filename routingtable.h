/*
 * routingtable.h
 *
 * Created: 10/09/2019 12:27:47
 *  Author: Mike
 */ 


#ifndef ROUTINGTABLE_H_
#define ROUTINGTABLE_H_

typedef uint8_t tNodeID;

void init_routingtable( void );
void addneighbor(tNodeID NodeID);
void removeneighbor(tNodeID NodeID);
tNodeID sendtowho(tNodeID);
uint8_t* GetRoutingString(uint8_t myID);
#endif /* ROUTINGTABLE_H_ */