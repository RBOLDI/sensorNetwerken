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

typedef struct pair {
	char* initials;
	uint8_t id;
} PAIR;

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
	nrfSetAutoAck(0);
	nrfEnableAckPayload();
	nrfEnableDynamicPayloads();

	nrfClearInterruptBits();
	nrfFlushRx();
	nrfFlushTx();

	PORTF.INT0MASK |= PIN6_bm;
	PORTF.PIN6CTRL  = PORT_ISC_FALLING_gc;
	PORTF.INTCTRL   = (PORTF.INTCTRL & ~PORT_INT0LVL_gm) | PORT_INT0LVL_LO_gc;

//	nrfOpenReadingPipe(0, global_pipe);
	nrfOpenReadingPipe(1, private_pipe);
	nrfOpenWritingPipe(private_pipe);
	nrfStartListening();
	
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	sei();
}

ISR(PORTF_INT0_vect){		//triggers when data is received
	uint8_t  tx_ds, max_rt, rx_dr;
	nrfWhatHappened(&tx_ds, &max_rt, &rx_dr);
	if(rx_dr){
		nrfRead(packet,  nrfGetDynamicPayloadSize());
		PORTF.DIRSET = PIN0_bm;
		PORTF.OUTTGL = PIN0_bm;
		printf("%s",packet);
	}
}

int main(void)
{
	int i = 0;
	init_stream(F_CPU);
	init_nrf();
    unsigned char nrfBuffer[32] = "kanker";
    /* Replace with your application code */
    while (1) 
    {
		nrfSend(nrfBuffer);
 		_delay_ms(1000);
    }
}

