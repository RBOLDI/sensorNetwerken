/*
 * messages.c
 *
 * Created: 11-9-2019 14:41:18
 *  Author: Steph
 */ 

#include <avr/io.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include "messages.h"
#include "nrf24L01.h"

uint8_t initials[NUMBER_OF_PREFIX_BYTES] = {0};
uint8_t message[MAX_MESSAGE_SIZE] = {0};
uint8_t fullMessage[FULL_MESSAGE_SIZE] = {0};
	
// Select Pipe to write to dependent on ID
uint8_t* pipe_selector(uint8_t ID){
	switch (ID){
		case 51:
		return FB_pipe;
		case 52:
		return RB_pipe;
		case 53:
		return SB_pipe;
		case 83:
		return MF_pipe;
		case 77:
		return JG_pipe;
		default:
		return 00;
	}
}
	
void sendMessage(){
	memmove(fullMessage, initials, NUMBER_OF_PREFIX_BYTES);
	memmove(fullMessage+NUMBER_OF_PREFIX_BYTES, message, MAX_MESSAGE_SIZE);

	printf("\r%s\n",(char *)fullMessage);
	
	PORTC.OUTSET = PIN0_bm;
	nrfSend( (uint8_t *) fullMessage);
	PORTC.OUTCLR = PIN0_bm;
	memset(message, 0 , sizeof(message));
	memset(fullMessage, 0, sizeof(fullMessage));
}

void sendPvtMessage(uint8_t targetID){
	nrfOpenWritingPipe(pipe_selector(targetID));
	sendMessage();
	nrfOpenWritingPipe(broadcast_pipe);
}