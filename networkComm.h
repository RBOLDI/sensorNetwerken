/*
 * networkComm.h
 *
 * Created: 30-9-2019 12:24:22
 *  Author: RBOLDI & YaBoii Steph
 */ 


#ifndef NETWORKCOMM_H_
#define NETWORKCOMM_H_

void init_PrivateComm(uint8_t _myid);
void sendPrivateMSG (uint8_t targetID, uint8_t *data);
uint8_t isMine(uint8_t MyID, uint8_t *data);
void ReceiveData(uint8_t MyID, uint8_t *data, uint8_t _size);

void (*sendMSG_Ptr)(uint8_t*, uint8_t, uint8_t*);

#endif /* NETWORKCOMM_H_ */

