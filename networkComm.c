/*
 * networkComm.c
 *
 * Created: 30-9-2019 12:23:56
 *  Author: RBOLDI
 */ 
/*
 * networkCommunication.c
 *
 * Created: 30-9-2019 12:10:03
 *  Author: Rowan
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
//----------------------------------
ReceiveData(uint8_t MyID, uint8_t *data){
	//Data header := BerichtType EigenID DoelID PakketLengte Hopcnt Data
	tNeighborHops BuurRoute;
	
	//Hopcnt met 1 verlagen
	data[4] --;
	// Check if Message is for me
	uint8_t recipiant = isMine(MyID, data);
	if(recipiant == 1){
		// If is for me load in Rpi ### MUST STILL BE ADDED ###
	}else{
		// If not check where to send
		BuurRoute = findLeastHops(recipiant);
		//Check least amount of hops to recipiant.
	}
}

uint8_t isMine(uint8_t MyID, uint8_t *data){
	if(data[2] == MyID) return 1;
	else return data[2];
}