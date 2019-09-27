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

typedef struct pair {
	char* initials;
	uint8_t id;
} PAIR;

const PAIR table[] =
{
	{ "FB_",  51 },
	{ "RB_",  52 },
	{ "SB_",  53 },
	{ "JG_",  77 },
	{ "AO_",  78 },
	{ "MF_",  83 },
};

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
	
void sendMessage(uint8_t targetID){
	memmove(fullMessage, initials, NUMBER_OF_PREFIX_BYTES);
	memmove(fullMessage+NUMBER_OF_PREFIX_BYTES, message, MAX_MESSAGE_SIZE);

	printf("\r%s\n",(char *)fullMessage);
	
	PORTC.OUTSET = PIN0_bm;
	nrfSend( (uint8_t *) fullMessage, MAX_MESSAGE_SIZE, pipe_selector(targetID));
	PORTC.OUTCLR = PIN0_bm;
	memset(message, 0 , sizeof(message));
	memset(fullMessage, 0, sizeof(fullMessage));
}

char* get_user_initials(uint8_t id)
{
	for (int i = 0; i < sizeof table / sizeof table[0]; ++i)
	{
		if(table[i].id == id)
		return table[i].initials;
	}
	return "XX_";		// Niet gevonden
}

void printf_hex(uint8_t *str, uint8_t str_len)
{
	for(int i = 0; i < str_len; i++)
	{
		printf("0x%02X ", str[i]);
	}
	printf("\n");
}

void printf_bin(uint8_t *str, uint8_t str_len)
{
	for (uint8_t i = 0; i <str_len; i++)
	{
		for (uint8_t j = 0; j < 8; j++)
		{
			printf("%d", ((str[i] & (0x80 >> j)) > 0));
		}
		printf(" ");
	}
	printf("\n");
}

void printf_Routing(uint8_t *str, uint8_t str_len)
{
	for(int i = 0; i < str_len; i++)
	{
		printf("%d ", str[i]);
	}
	printf("\n");
}