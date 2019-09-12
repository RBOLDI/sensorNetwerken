/*
 * routingtable.h
 *
 * Created: 10/09/2019 12:27:47
 *  Author: Mike
 */ 


#ifndef ROUTINGTABLE_H_
#define ROUTINGTABLE_H_

void copyrow( uint8_t idrow, uint8_t* row );
void addneighbor( uint8_t idneighbor );
void removeneighbor( uint8_t idneighbor );
uint8_t* getownlist( void );
uint8_t* getotherlist( uint8_t idsensornode );
void init_routingtable( uint8_t uNodeID );

#endif /* ROUTINGTABLE_H_ */