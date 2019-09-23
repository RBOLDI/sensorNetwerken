/*
 * routingtable.h
 *
 * Created: 10/09/2019 12:27:47
 *  Author: Mike
 */ 


#ifndef ROUTINGTABLE_H_
#define ROUTINGTABLE_H_

void init_RoutingTable(uint8_t uMyID);
void addKnownNode(uint8_t uNodeID );
uint8_t isKnown(uint8_t uNodeID);
void addNeighbor(uint8_t uNodeID);
void removeNeighbor(uint8_t uNodeID);
uint8_t isNeighbor(uint8_t uNodeID);
#endif /* ROUTINGTABLE_H_ */