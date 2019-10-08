
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "networkComm.h"
#include "routingtable.h"
#include "messages.h"
#include "nrf24L01.h"

void (*sendMSG_Ptr)(uint8_t*, uint8_t, uint8_t*);
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
	memset(aPrivateSendString, 0, 32);
	
	aPrivateSendString[0] = DHDR;
	aPrivateSendString[1] = MyID;
	aPrivateSendString[2] = targetID;
	aPrivateSendString[3] = messageInfo.uHops;
	
	for(uint8_t i = 0; i < SENSORDATALENGTH; i++)
	{
		aPrivateSendString[i+4] = data[i];
	}
	sendMSG_Ptr(aPrivateSendString, (SENSORDATALENGTH+4), pipe_selector(targetID));
}

//Function checks if privately received data is meant for me
// if not it calculates the least hopes to the recipiant and sends 
// message to first node in that path.
void ReceiveData(uint8_t *_data, uint8_t _size) //Get size from global int PayloadSize in main.c 
{ 
	tNeighborHops BuurRoute;
	_data[3]--;
	
	if(_data[2] == MyID)
	{
		// If is for me load in Rpi ### MUST STILL BE ADDED ###
	}else{
		BuurRoute = findLeastHops(_data[2]);
		sendMSG_Ptr(_data, _size, pipe_selector(BuurRoute.uNeighbor));
	}
}
