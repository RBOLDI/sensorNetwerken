/*
 * GccApplication1.c
 *
 * Created: 3-9-2019 07:28:30
 * Author : Rowan
 */ 
#define F_CPU 2000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include "nrf24L01.h"
#include "nrf24spiXM2.h"
#include "stream.h"
#include "uart.h"

#include "KeyboardCodes.h"

#define TRANSMITTER (PORTD.IN & PIN0_bm)
#define RECEIVER !TRANSMITTER
#define MYID 51

#define FULL_MESSAGE_SIZE 32
#define NUMBER_OF_PREFIX_BYTES 3
#define MAX_MESSAGE_SIZE FULL_MESSAGE_SIZE - NUMBER_OF_PREFIX_BYTES // Waarvan de laatste is '\0'

void writeMessage();

typedef struct pair {
	char* initials;
	uint8_t id;
} PAIR;

uint8_t newDataFlag = 0;
uint8_t sendDataFlag = 0;

const PAIR table[] =
{
	{ "FB_",  51 },
	{ "RB_",  52 },
	{ "SB_",  53 },
	{ "JG_",  77 },
	{ "AO_",  78 },
};

char* get_user_initials(uint8_t id)
{
	for (int i = 0; i < sizeof table / sizeof table[0]; ++i)
	{
		if(table[i].id == id)
		return table[i].initials;
	}
	return "XX_";
}

void init_nrf(void){
	nrfspiInit();
	nrfBegin();

	nrfSetRetries(NRF_SETUP_ARD_1000US_gc, NRF_SETUP_ARC_8RETRANSMIT_gc);
	nrfSetPALevel(NRF_RF_SETUP_PWR_6DBM_gc);
	nrfSetDataRate(NRF_RF_SETUP_RF_DR_250K_gc);
	nrfSetCRCLength(NRF_CONFIG_CRC_16_gc);
	nrfSetChannel(channel);
	nrfSetAutoAck(1);
	nrfEnableAckPayload();
	nrfEnableDynamicPayloads();

	nrfClearInterruptBits();
	nrfFlushRx();
	nrfFlushTx();

	PORTF.INT0MASK |= PIN6_bm;
	PORTF.PIN6CTRL  = PORT_ISC_FALLING_gc;
	PORTF.INTCTRL   = (PORTF.INTCTRL & ~PORT_INT0LVL_gm) | PORT_INT0LVL_LO_gc;

	PORTF.DIRSET = PIN0_bm;
	
	PORTD.DIRCLR = PIN0_bm;
	PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc;

//	nrfOpenReadingPipe(0, global_pipe);
	nrfOpenReadingPipe(1, private_pipe);
	nrfOpenWritingPipe(private_pipe);
	nrfStartListening();
	
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	sei();
}

ISR(PORTF_INT0_vect){		//triggers when data is received
	uint8_t  tx_ds, max_rt, rx_dr;
	memset(packet, 0, sizeof(packet));
	nrfWhatHappened(&tx_ds, &max_rt, &rx_dr);
	if(rx_dr){
		nrfRead(packet, nrfGetDynamicPayloadSize());
		newDataFlag = 1;
	}
}

int main(void)
{
	init_stream(F_CPU);
	init_nrf();
    char initials[NUMBER_OF_PREFIX_BYTES] = {0};
	char message[MAX_MESSAGE_SIZE] = {0};
	char fullMessage[FULL_MESSAGE_SIZE] = {0};
	
    while (1) 
    {
		if(RECEIVER && newDataFlag)
		{
	   		newDataFlag = 0;
			PORTF.OUTTGL = PIN0_bm;
			printf("%s\n", packet);
		}
		else if(TRANSMITTER)
		{
			memset(initials, 0, sizeof(initials));
			memset(message, 0, sizeof(message));
			memset(fullMessage, 0, sizeof(fullMessage));
			
			
			memcpy(&initials, get_user_initials(MYID), NUMBER_OF_PREFIX_BYTES);
			writeMessage(&message);
			strcat(fullMessage,strcat(initials,message));

			printf("\r%s%s\n",initials,message);
			
			if(sendDataFlag)
			{
				sendDataFlag = 0;
				nrfSend( (uint8_t *) fullMessage);		// Initialen moeten er nog voor worden geplakt. strcat is kapot irritant en wil niet goed werken
			}
		}
    }
}

void writeMessage(char* msg){
	int pos = 0;
	char c_byte[MAX_MESSAGE_SIZE] = {0};
	char c;
	
	while(1) {
		if((c = uart_fgetc(&uart_stdinout)) == UART_NO_DATA){
			continue;
		}
		else if (c == ENTER || pos == (MAX_MESSAGE_SIZE-1))
		{
			c_byte[pos] = '\0';
			sendDataFlag = 1;
			
			strcpy(msg,c_byte);
			return;
		}
		else if (c == BACKSPACE)
		{
			pos--;
			c_byte[pos] = 0;
			printf("\b \b");
		}
		else
		{
			printf("%c", c);
			c_byte[pos] = c;
			pos++;
		}
	}	
}