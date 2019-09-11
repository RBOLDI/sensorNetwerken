/*
 * messages.c
 *
 * Created: 11-9-2019 14:41:18
 *  Author: Steph
 */ 
#define F_CPU 2000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include "messages.h"
#include "nrf24L01.h"

uint8_t initials[NUMBER_OF_PREFIX_BYTES] = {0};
uint8_t message[MAX_MESSAGE_SIZE] = {0};
uint8_t fullMessage[FULL_MESSAGE_SIZE] = {0};
	
void sendMessage(){
	memmove(fullMessage, initials, NUMBER_OF_PREFIX_BYTES);
	memmove(fullMessage+NUMBER_OF_PREFIX_BYTES, message, MAX_MESSAGE_SIZE);

	printf("\r%s\n",(char *)fullMessage);
	
	PORTC.OUTSET = PIN0_bm;
	nrfSend( (uint8_t *) fullMessage);		// Initialen moeten er nog voor worden geplakt. strcat is kapot irritant en wil niet goed werken
	PORTC.OUTCLR = PIN0_bm;
	memset(message, 0 , sizeof(message));
	memset(fullMessage, 0, sizeof(fullMessage));
}

void sendPvtMessage(uint8_t targetID){
	
	nrfOpenWritingPipe(pipe_selector(targetID));
	
	//Make the target target pipe the Rx0 pipe (verander volgorde)
	//nrfOpenReadingPipe(0,pipe_selector(targetID));
	//nrfOpenReadingPipe(1,broadcast_pipe);
	
	//Send message.
	sendMessage();
	//_delay_ms(500);
	//Revert changes
	//nrfOpenReadingPipe(1,pipe_selector(targetID));
	//nrfOpenReadingPipe(0,broadcast_pipe);
	nrfOpenWritingPipe(broadcast_pipe);
}