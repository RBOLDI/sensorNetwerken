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

#define TRANSMITTER (PORTD.IN & PIN0_bm)
#define RECEIVER !TRANSMITTER


#define FULL_MESSAGE_SIZE 32
#define NUMBER_OF_PREFIX_BYTES 3
#define MAX_MESSAGE_SIZE FULL_MESSAGE_SIZE - NUMBER_OF_PREFIX_BYTES // Waarvan de laatste is '\0'

//Function prototypes
const uint8_t getID();
void writeMessage();
uint8_t* pipe_selector(uint8_t ID);

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
	return "User not found";
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

	PORTF.DIRSET = PIN0_bm;
	
	PORTD.DIRCLR = PIN0_bm, PIN1_bm, PIN2_bm, PIN3_bm;
	
	PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc;

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
	memset(packet, NULL, sizeof(packet));
	nrfWhatHappened(&tx_ds, &max_rt, &rx_dr);
	if(rx_dr){
		nrfRead(packet, nrfGetDynamicPayloadSize());
		newDataFlag = 1;
	}
}

int main(void)
{
	init_stream(F_CPU);
	
	//Set ID selector pins
	PORTD.DIRCLR = PIN1_bm, PIN2_bm, PIN3_bm;
	
	PORTD.PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN3CTRL = PORT_OPC_PULLUP_gc;
	
	const uint8_t MYID = getID();
	
	
	init_nrf(MYID);
    uint8_t initials[NUMBER_OF_PREFIX_BYTES] = {0};
	uint8_t message[MAX_MESSAGE_SIZE] = {0};
	uint8_t fullMessage[FULL_MESSAGE_SIZE] = {0};
	
    /* Replace with your application code */
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
			writeMessage(&message);

			printf("\r%s%s\n",get_user_initials(MYID),message);
			
			if(sendDataFlag)
			{
				sendDataFlag = 0;
				nrfSend(message);		// Initialen moeten er nog voor worden geplakt. strcat is kapot irritant en wil niet goed werken
			}
		}
    }
}

void writeMessage(uint8_t* msg){
	int pos = 0;
	uint8_t c_byte[MAX_MESSAGE_SIZE] = {0};
	uint16_t c;
	
	while(1) {
		if((c = uart_fgetc(&uart_stdinout)) == 0x0100){
			continue;
		}
		else if (c == '\r' || pos == (MAX_MESSAGE_SIZE-1))
		{
			c_byte[pos] = '\0';
			sendDataFlag = 1;
			
			strcpy(msg,c_byte);
			return;
		}
		else if (c == '\b')
		{
			pos--;
			c_byte[pos] = NULL;
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