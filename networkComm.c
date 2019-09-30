/*
 * networkComm.c
 *
 * Created: 30-9-2019 12:23:56
 *  Author: RBOLDI & YaBoii Steph
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "routingtable.h"
#include "messages.h"
#include "nrf24L01.h"

#define PRIVATE_MSG 0x03

uint8_t *aPrivateSendString = NULL;
uint8_t MyID = NULL;
uint8_t sensorDataLenght = 2;

void init_PrivateComm(uint8_t _myid)
{
		aPrivateSendString			= (uint8_t *) calloc(32, sizeof(uint8_t));
		MyID = _myid;
}

void sendPrivateMSG (uint8_t targetID, uint8_t *data)
{
	private_pipe[4] = targetID;
	tNeighborHops messageInfo = findLeastHops(targetID);
	memset(aPrivateSendString, 0, 32);
	
	aPrivateSendString[0] = PRIVATE_MSG;
	aPrivateSendString[1] = MyID;
	aPrivateSendString[2] = targetID;
	aPrivateSendString[3] = messageInfo.uHops;
	
	for(uint8_t i = 0; i < sensorDataLenght; i++)
	{
		aPrivateSendString[i+4] = data[i];
	}
	nrfSendMessage(aPrivateSendString, (sensorDataLenght+4), private_pipe);
}


// -- Functions for receiving data--

uint8_t isMine(uint8_t _myid, uint8_t *_data)
{
	if(_data[2] == _myid) return 1;
	else return _data[2];
}

//Function checks if privately received data is meant for me
// if not it calculates the least hopes to the recipiant and sends 
// message to first node in that path.
void ReceiveData(uint8_t _myid, uint8_t *_data, uint8_t _size) //Get size from global int PayloadSize in main.c 
{ 
	uint8_t recipiant = isMine(_myid, _data);
	tNeighborHops BuurRoute;
	_data[4] --;
	
	if(recipiant == 1)
	{
		// If is for me load in Rpi ### MUST STILL BE ADDED ###
	}else{
		BuurRoute = findLeastHops(recipiant);
		nrfSendMessage(_data, _size, pipe_selector(BuurRoute.uNeighbor)); 
	}
}
