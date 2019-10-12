#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <util/atomic.h>
#include <stdbool.h>

#include "networkComm.h"
#include "nrf24L01.h"
#include "avr_compiler.h"
#include "messages.h"
#include "routingtable.h"

uint8_t *aPrivateSendString = NULL;
uint8_t MyID = 0;

void init_PrivateComm(uint8_t _myid)
{
		aPrivateSendString = (uint8_t *) calloc(32, sizeof(uint8_t));
		MyID = _myid;
}

void sendPrivateMSG (uint8_t targetID, uint8_t *data)
{
	tNeighborHops messageInfo = findLeastHops(targetID);
	if(messageInfo.uNeighbor != 0){
		memset(aPrivateSendString, 0, 32);
		
		aPrivateSendString[0] = DATAHEADER;
		aPrivateSendString[1] = MyID;
		aPrivateSendString[2] = targetID;
		aPrivateSendString[3] = messageInfo.uHops;
		
		for(uint8_t i = 0; i < SENSORDATALENGTH; i++)
		{
			aPrivateSendString[i+4] = data[i];
		}
		
		nrfSendMessage(aPrivateSendString, (SENSORDATALENGTH + 4), pipe_selector(messageInfo.uNeighbor), true);
	}
}

//Function checks if privately received data is meant for me
// if not it calculates the least hopes to the recipiant and sends 
// message to first node in that path.
void ReceiveData(uint8_t *_data, uint8_t _size) //Get size from global int PayloadSize in main.c 
{ 
	if(_data[2] == MyID)
	{
		printf("Data is for me\n");
		// If is for me load in Rpi ### MUST STILL BE ADDED ###
	}
	else if (--_data[3] > 0)
	{
		tNeighborHops BuurRoute = findLeastHops(_data[2]);
		nrfSendMessage(_data, _size, pipe_selector(BuurRoute.uNeighbor), true);
		printf("Data is for");
        printf("%d\n",BuurRoute.uNeighbor);
	}
}

void nrfSendMessage(uint8_t *str, uint8_t str_len, uint8_t *pipe, bool ack)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON);
	PORTC.OUTSET = PIN0_bm;
	nrfStopListening();
	nrfOpenWritingPipe(pipe);
	delay_us(130);
	if (ack)
	{
		nrfStartWrite(str, str_len, NRF_W_TX_PAYLOAD);
	}
	else 
	{
		nrfStartWrite(str, str_len, NRF_W_TX_PAYLOAD_NO_ACK);
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE);
}