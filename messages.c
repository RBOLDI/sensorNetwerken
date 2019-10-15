/*
 * messages.c
 *
 * Created: 11-9-2019 14:41:18
 *  Author: Steph
 */ 

#include <stdio.h>

#include "messages.h"
#include "nrf24L01.h"
#include "debug_opts.h"
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
	printf("\r\n");
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
	printf("\r\n");
}

void printf_Routing(uint8_t *str, uint8_t str_len)
{
	for(int i = 0; i < str_len; i++)
	{
		DB_MSG(("%d ", str[i]));
	}
	DB_MSG(("\r\n"));
}

void printf_DeviceSerial(uint8_t *str, uint8_t str_len)
{
	DB_MSG(("|---------------DeviceSerial--------------|\r\n"));

	for(int i = 0; i < str_len-1; i++)
	{
		DB_MSG(("%d-", str[i]));
	}
		DB_MSG(("%d\r\n",str[str_len-1]));

	DB_MSG(("|---------------DeviceSerial--------------|\r\n"));
}

void printf_SetColor(char* cc)
{
	if(MYID != BASESTATION_ID) printf("%s",cc);
}