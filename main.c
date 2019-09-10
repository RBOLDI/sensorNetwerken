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

#define		FB !(PORTD.IN & PIN1_bm)
#define		RB !(PORTD.IN & PIN2_bm)
#define		SB !(PORTD.IN & PIN3_bm)
#include "KeyboardCodes.h"


#define FULL_MESSAGE_SIZE 32
#define NUMBER_OF_PREFIX_BYTES 3
#define MAX_MESSAGE_SIZE FULL_MESSAGE_SIZE - NUMBER_OF_PREFIX_BYTES // Waarvan de laatste is '\0'

//Function prototypes
const uint8_t getID();
uint8_t* pipe_selector(uint8_t ID);
uint8_t writeMessage();

typedef struct pair {
	char* initials;
	uint8_t id;
} PAIR;

uint8_t newDataFlag = 0;
uint8_t sendDataFlag = 0;
uint8_t newKeyboardData = 0;

int pointer = 0;
char charBuffer[MAX_MESSAGE_SIZE] = {0};


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
	return "XX_";		// Niet gevonden
}

void init_io(void){
	PORTF.DIRSET = PIN0_bm | PIN1_bm;
	PORTC.DIRSET = PIN0_bm;
	
	//Set ID selector pins
	PORTD.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
	
	PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN3CTRL = PORT_OPC_PULLUP_gc;
}
void init_nrf(const uint8_t pvtID){
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

	//TODO: automatisch reading pipes selecteren, zodat eigen adres hier niet meer tussen staat.
	//Reading pipe 0 dient gelijk te zijn aan de Writing pipe (xMega ID) i.v.m. auto acknowledge.
	 
	//nrfOpenReadingPipe(0, global_pipe);
	nrfOpenReadingPipe(0, pipe_selector(pvtID));
	nrfOpenReadingPipe(1, FB_pipe);
	nrfOpenReadingPipe(2, RB_pipe);
	nrfOpenReadingPipe(3, SB_pipe);
	nrfStartListening();
	
	nrfOpenWritingPipe(pipe_selector(pvtID));
	
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
	

	
	const uint8_t MYID = getID();
	
	init_nrf(MYID);
    
	uint8_t initials[NUMBER_OF_PREFIX_BYTES] = {0};
	memmove(initials, get_user_initials(MYID), NUMBER_OF_PREFIX_BYTES);
	uint8_t message[MAX_MESSAGE_SIZE] = {0};
	uint8_t fullMessage[FULL_MESSAGE_SIZE] = {0};
	
    while (1) 
    {
		if(newDataFlag)
		{
	   		newDataFlag = 0;
			printf("%s\n", packet);
			PORTF.OUTTGL = PIN0_bm;
		}

		if(newKeyboardData)
		{
			newKeyboardData = 0;
			writeMessage(&message);
		}
		
		if(sendDataFlag)
		{
			sendDataFlag = 0;
			
			memmove(fullMessage, initials, NUMBER_OF_PREFIX_BYTES);
			memmove(fullMessage+NUMBER_OF_PREFIX_BYTES, message, MAX_MESSAGE_SIZE);

			printf("\r%s\n",fullMessage);
				
			PORTC.OUTSET = PIN0_bm;
			nrfSend( (uint8_t *) fullMessage);		// Initialen moeten er nog voor worden geplakt. strcat is kapot irritant en wil niet goed werken
			PORTC.OUTCLR = PIN0_bm;

			memset(message, 0 , sizeof(message));
			memset(fullMessage, 0, sizeof(fullMessage));
		}
    }
}

uint8_t writeMessage(char* msg){
	char c = uart_fgetc(&uart_stdinout);
	
	if (c == ENTER)
	{
		charBuffer[pointer] = '\0';
		sendDataFlag = 1;
		
		strcpy(msg,charBuffer);
		
		pointer = 0;
		memset(charBuffer, 0, sizeof(charBuffer));
		
		return 1;
	}
	else if (c == BACKSPACE)
	{
		if(pointer > 0)
		{
			charBuffer[pointer--] = 0;
			printf("\b \b");
		}
	}
	else if (pointer < (MAX_MESSAGE_SIZE - 1))
	{
		printf("%c", c);
		charBuffer[pointer++] = c;
	}
	
	return 0;
}

uint8_t* pipe_selector(uint8_t ID){
	switch (ID){
		case 51:  
			return FB_pipe;
		case 52: 
			return RB_pipe;
		case 53:
			return SB_pipe;
	}
}

const uint8_t getID(){
	if(FB) return 51;
	else if(RB) return 52;
	else if(SB) return 53;
	else return 00;
}