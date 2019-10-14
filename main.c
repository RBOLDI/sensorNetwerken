/*
 * SensorNetwerkenNox.c
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
#include <util/atomic.h>
#include <stdbool.h>
#include "clksys_driver.h"
#include "nrf24L01.h"
#include "nrf24spiXM2.h"
#include "stream.h"
#include "uart.h"
#include "powersaving.h"
#include "routingtable.h"
#include "messages.h"
#include "debug_opts.h" 
#include "serialnumber.h"
#include "ADC.h"
#include "networkComm.h"

//Function prototypes
void init_nrf(const uint8_t pvtID);
void SendRouting( void );
void bootFunction(void);
void parseIncomingData(void);

volatile uint8_t newDataFlag		= 0;
volatile uint8_t newBroadcastFlag	= 0;
volatile uint8_t successTXFlag		= 0;
volatile uint8_t maxRTFlag			= 0;
volatile uint8_t sampleCounter		= 0;

uint8_t MYID;
uint8_t device_serial[11];
uint8_t PayloadSize;
uint8_t TXCounter = 0;

enum states {
	S_Boot,
	S_SendRouting,
	S_SendSensorData,
	S_Idle,
	S_GotMail,
	S_WaitforTX,
};

enum states currentState, nextState = S_Boot;

ISR(TCE0_OVF_vect)
{
	PORTF.OUTSET = PIN1_bm;
	updateNeighborList();
	newBroadcastFlag = 1;
	ADC_timer();
}

ISR(PORTD_INT0_vect)
{
	PORTF.OUTSET = PIN1_bm;
	newBroadcastFlag = 1;
}

ISR(PORTF_INT0_vect){
	uint8_t status;
	status = nrfReadRegister(REG_STATUS);

	if(status & NRF_STATUS_RX_DR_bm)			// RX Data Ready
	{
		PORTF.OUTSET = PIN0_bm;
		newDataFlag = 1;
	}

	if(status & NRF_STATUS_TX_DS_bm)			// TX Data Sent
	{
		successTXFlag = 1;
	}

	if(status & NRF_STATUS_MAX_RT_bm)			// Max Retries
	{
		maxRTFlag = 1;
	}
}

int main(void)
{
	while (1)
	{
		switch(currentState) {
			case S_Boot:
				bootFunction();
				printf("S_Boot\n");
				nextState = S_SendRouting;
			break;
			case S_SendRouting:
				printf("S_SendRouting\n");
				SendRouting();
				PORTF.OUTCLR = PIN1_bm;
				nextState = S_WaitforTX;
			break;
			case S_SendSensorData:
				printf("S_SendSensorData\n");
				sendPrivateMSG (105, sampleData);
				PORTF.OUTCLR = PIN1_bm;
				nextState = S_WaitforTX;
			break;
			case S_WaitforTX:
				if(successTXFlag) {
					TXCounter = 0;
					if ( nrfReadRegister(REG_FIFO_STATUS) & NRF_FIFO_STATUS_TX_EMPTY_bm )
					{
						nrfWriteRegister(REG_STATUS, NRF_STATUS_TX_DS_bm );
						nrfStartListening();
						PORTC.OUTCLR = PIN0_bm;
						TCD0.CTRLFSET = TC_CMD_RESTART_gc;
						successTXFlag = 0;
						printf("Succesfull TX\n");
						nextState = S_Idle;
					}
					
				}
				else if(maxRTFlag) {
					TXCounter = 0;
					nrfWriteRegister(REG_STATUS, NRF_STATUS_MAX_RT_bm );
					nrfFlushTx();
					TCD0.CTRLFSET = TC_CMD_RESTART_gc;
					nrfStartListening();
					_delay_us(130);
					PORTC.OUTCLR = PIN0_bm;
					maxRTFlag = 0;
					printf("Max Retries\n");
					nextState = S_Idle;
				}
				else if ( TXCounter > 250 )
				{
					TXCounter = 0;
					nrfWriteRegister(REG_STATUS, NRF_STATUS_MAX_RT_bm | NRF_STATUS_TX_DS_bm );
					nrfFlushTx();
					TCD0.CTRLFSET = TC_CMD_RESTART_gc;
					nrfStartListening();
					_delay_us(130);
					PORTC.OUTCLR = PIN0_bm;
					printf("TX Failed\n");
					nextState = S_Idle;
				}
				else 
				{
					TXCounter++;
				}
			break;
			case S_GotMail:
				printf("S_GotMail\n");
				parseIncomingData();
				PORTF.OUTCLR = PIN0_bm;
				nextState = S_Idle;
			break;
			case S_Idle:
				idle();
				if(newBroadcastFlag) {
					newBroadcastFlag = 0;
					nextState = S_SendRouting;
				}
				else if ( ADC_sample() && MYID != 105 )
				{
					nextState = S_SendSensorData;
				}
				else if(newDataFlag) {
					ATOMIC_BLOCK(ATOMIC_FORCEON);
					memset(packet.content, 0, sizeof(packet.content));
					packet.size = nrfGetDynamicPayloadSize();
					if ( nrfRead(packet.content, packet.size) )
					{
						nrfWriteRegister(REG_STATUS, NRF_STATUS_RX_DR_bm );
						newDataFlag = 0;
					}
					ATOMIC_BLOCK(ATOMIC_RESTORESTATE);
					nextState = S_GotMail;
				}
				else {
					nextState = S_Idle;
				}
			break;
			default:
					nextState = S_Idle;
			break;
			}
		currentState = nextState;
	}
}
void SendRouting( void )
{
	uint8_t *str = getRoutingString();
	nrfSendMessage(str, str[2], broadcast_pipe, false);
}

/* This function will be called when state equals S_Boot.
	It will run all the initializations of the Xmega. Including I/O,
	setting MYID, UART stream and nRF */
void bootFunction(void)
{
	InitClocks();
	init_io();
	init_stream(F_CPU);

	NVM_GetDeviceSerial(device_serial);
	MYID = GetIdFromLookup(device_serial);

	init_nrf(MYID);
	
	init_RoutingTable(MYID);
	init_PrivateComm(MYID);
	init_lowpower();
	init_adc();
	ADC_timer();
	
	DB_MSG("\n----Debug mode enabled----\n\n");
	printf_DeviceSerial(device_serial,11);

	_delay_ms(200);
}

/* This function will be called when state equals S_GotMail.
	It will parse the message to determine what kind of message 
	it is, and what to do with it. UMT means Unknown Message Type */
void parseIncomingData( void )
{
	switch(packet.content[0])
	{
		case ROUTINGHEADER:
			addNeighbor(packet.content[1]);
			printf_Routing(packet.content, packet.size);
			FillRoutingTable(packet.content, packet.size);
		break;
		case DATAHEADER:
			DB_MSG("Received Data");
			ReceiveData(packet.content, packet.size);	
			printf("0x%02X %d %d\n", packet.content[0], packet.content[1], (( (uint16_t) packet.content[2] ) << 8) | packet.content[3]);
		break;
		default:
		 	printf("UMT: ");
			printf_hex(packet.content, sizeof(packet.content));
		break;
	}
}

void init_nrf(const uint8_t pvtID){
	nrfspiInit();
	nrfSetRetries(NRF_SETUP_ARD_1000US_gc,	NRF_SETUP_ARC_10RETRANSMIT_gc);
	nrfSetPALevel(NRF_RF_SETUP_PWR_18DBM_gc);
	nrfSetDataRate(NRF_RF_SETUP_RF_DR_250K_gc);
	nrfSetCRCLength(NRF_CONFIG_CRC_16_gc);
	nrfSetChannel(channel);
	nrfSetAutoAck(0);
	nrfSetAutoAckPipe(1, true);
	nrfEnableDynamicPayloads();
	nrfEnableAckPayload();
	
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