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
#include "powersaving.h"
#include "routingtable.h"
#include "messages.h"

#define		FB !(PORTD.IN & PIN1_bm)
#define		RB !(PORTD.IN & PIN2_bm)
#define		SB !(PORTD.IN & PIN3_bm)
#define		MF !(PORTD.IN & PIN4_bm)
#define		JG !(PORTD.IN & PIN5_bm)

//Function prototypes
const uint8_t getID();
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
	{ "MF_",  83 },
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

void init_nrf(const uint8_t pvtID){
	nrfspiInit();
	nrfBegin();

	nrfSetRetries(NRF_SETUP_ARD_1000US_gc,	NRF_SETUP_ARC_10RETRANSMIT_gc);
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

	//Starts in broadcast mode with own pvt ID selected by HW pin.  
	
	nrfOpenReadingPipe(0, broadcast_pipe);
	nrfOpenReadingPipe(1, pipe_selector(pvtID));
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

void broadcast_startup(uint8_t id)
{
	uint8_t msg[NUMBER_OF_PREFIX_BYTES] = {'b',id,'\0'};
	
	nrfStopListening();

	nrfOpenWritingPipe(broadcast_pipe);
	
	nrfWrite((uint8_t *) msg, NUMBER_OF_PREFIX_BYTES);

	nrfStartListening();
	
}
int main(void)
{
	init_io();

	const uint8_t MYID = getID();
	memmove(initials, get_user_initials(MYID), NUMBER_OF_PREFIX_BYTES);

	init_stream(F_CPU);

	init_nrf(MYID);

 	_delay_ms(200);

   	broadcast_startup(MYID);


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
			// ** Test code for testing pvt message ** // 
			//sendMessage();
			sendPvtMessage(51);
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

const uint8_t getID(){
	if(FB) return 51;
	else if(RB) return 52;
	else if(SB) return 53;
	else if(MF) return 83;
	else if(JG) return 77;
	else return 00;
}

