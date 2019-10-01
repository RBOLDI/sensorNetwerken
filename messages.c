/*
 * messages.c
 *
 * Created: 11-9-2019 14:41:18
 *  Author: Steph
 */ 

#include <avr/io.h>
#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "messages.h"
#include "nrf24L01.h"

uint8_t message[MAX_MESSAGE_SIZE] = {0};
uint8_t fullMessage[FULL_MESSAGE_SIZE] = {0};


// Select Pipe to write to dependent on ID
uint8_t* pipe_selector(uint8_t ID){
	private_pipe[0] = ID;
	return private_pipe;
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

void printf_DeviceSerial(uint8_t *str, uint8_t str_len)
{
	printf("|---------------DeviceSerial--------------|\n");

	for(int i = 0; i < str_len-1; i++)
	{
		printf("%d-", str[i]);
	}
	printf("%d\n",str[str_len-1]);

	printf("|---------------DeviceSerial--------------|\n");
}