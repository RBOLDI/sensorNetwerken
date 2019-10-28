

#ifndef NETWORKCOMM_H_
#define NETWORKCOMM_H_

#define SENSORDATALENGTH 2
#define KEY 0b01100010

void init_PrivateComm(uint8_t _myid);
void sendPrivateMSG (uint8_t targetID, uint8_t *data);
uint8_t ReceiveData(uint8_t *data, uint8_t _size);
void nrfSendMessage(uint8_t *str, uint8_t str_len, uint8_t *pipe, bool ack);
#endif /* NETWORKCOMM_H_ */
