/*
 * networkComm.h
 *
 * Created: 30-9-2019 12:24:22
 *  Author: Rowan
 */ 


#ifndef NETWORKCOMM_H_
#define NETWORKCOMM_H_

void init_PrivateComm(uint8_t _myid);
void sendPrivateMSG (uint8_t targetID, uint8_t *data);


#endif /* NETWORKCOMM_H_ */