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
#include <avr/interrupt.h>
#include "nrf24L01.h"
#include "nrf24spiXM2.h"
#include "stream.h"
#include "uart.h"


#define TRANSMITTER (PORTD.IN & PIN0_bm)
#define RECEIVER !TRANSMITTER
#define MYID 51

int writeMessage();

typedef struct pair {
	char* initials;
	uint8_t id;
} PAIR;

uint8_t newDataFlag = 0;
uint8_t sendDataFlag = 0;

const PAIR table[] =
{
	{ "FB",  51 },
	{ "RB",  52 },
	{ "SB",  53 },
	{ "JG",  77 },
	{ "AO",  78 },
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
	init_nrf();
    uint8_t nrfBuffer[32] = "Dit is een lang bericht ja toc\n";
    /* Replace with your application code */
    while (1) 
    {
		if(newDataFlag)
		{
	   		newDataFlag = 0;
			PORTF.OUTTGL = PIN0_bm;
			printf("%s\n", packet);
		}
			writeMessage(&nrfBuffer);
		if(sendDataFlag)
		{
			sendDataFlag = 0;
			nrfSend(nrfBuffer);
		}
	}
}


int writeMessage(uint8_t* message){
	int pos = 0;
	uint8_t c_byte[32] = {0};
	uint16_t c;
	int rc = 0;
	while(rc == 0) {
		if((c = uart_fgetc(&uart_stdinout)) == 0x0100){
			continue;
		}
		else if (c == '\r')
		{
			strcpy(message,c_byte);
			memset(c_byte, NULL, sizeof(c_byte));
			
			sendDataFlag = 1;
			rc = 1;
		}
		else if (c == '\b')
		{
			pos--;
			c_byte[pos] = NULL;
			printf("\b \b");
		}
		else if(c == '*'){
			clear_screen();
			return rc;
		}
		else
		{
			printf("%c", c);
			c_byte[pos] = c;
			pos++;
		}
	}

	printf("\r%s_%s\n", get_user_initials(MYID), message);	// Print een regel met initialen + het opgestelde bericht.
	
	return rc;
}