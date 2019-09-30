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

#define		FB !(PORTD.IN & PIN1_bm)
#define		RB !(PORTD.IN & PIN2_bm)
#define		SB !(PORTD.IN & PIN3_bm)
#define		MF !(PORTD.IN & PIN4_bm)
#define		JG !(PORTD.IN & PIN5_bm)

#define		BROADCAST	0x01
#define		RHDR		0x02
#define		DHDR		0x03
#define		BCREPLY		0x04

//Function prototypes
void init_nrf(const uint8_t pvtID);
void nrfSendMessage(uint8_t *str, uint8_t str_len, uint8_t *pipe);
void SendRouting( void );
void bootFunction(void);
void parseIncomingData(void);
const uint8_t getID();


uint8_t newDataFlag = 0;
uint8_t sendDataFlag = 0;
uint8_t newBroadcastFlag = 0;
uint8_t successTXFlag = 0;
uint8_t maxRTFlag = 0;


uint8_t MYID;
uint8_t device_serial[11];

enum states {
	S_Boot,
	S_SendRouting,
	S_Idle,
	S_GotMail,
	S_Send,
};

enum states currentState, nextState = S_Boot;

char charBuffer[MAX_MESSAGE_SIZE] = {0};

ISR(TCE0_OVF_vect)
{
	PORTF.OUTTGL = PIN1_bm;
	updateNeighborList();
	newBroadcastFlag = 1;
}

ISR(PORTD_INT0_vect)
{
	PORTF.OUTTGL = PIN1_bm;
	newBroadcastFlag = 1;
}

ISR(PORTF_INT0_vect){		//triggers when data is received
	uint8_t  tx_ds, max_rt, rx_dr;
	nrfWhatHappened(&tx_ds, &max_rt, &rx_dr);
	if(rx_dr){
		PORTF.OUTTGL = PIN0_bm;
		memset(packet, 0, sizeof(packet));
		nrfRead(packet, nrfGetDynamicPayloadSize());
		newDataFlag = 1;
	}
	if(tx_ds) successTXFlag = 1;
	if(max_rt) maxRTFlag = 1;
}

int main(void)
{
	while (1)
	{
		switch(currentState) {
			case S_Boot:
			bootFunction();
			printf("S_Boot\n");
			nextState = S_Idle;
			break;
			case S_SendRouting:
			printf("S_SendRouting\n");
			SendRouting();
			_delay_ms(5);
			nextState = S_Idle;
			break;
			case S_GotMail:
			printf("S_GotMail\n");
			parseIncomingData();
			nextState = S_Idle;
			break;
			case S_Idle:
			idle();
			if(newBroadcastFlag) {
				newBroadcastFlag = 0;
				nextState = S_SendRouting;
			}
			else if(newDataFlag) {
				newDataFlag = 0;
				nextState = S_GotMail;
			}
			else if(successTXFlag) {
				printf("Succesfull TX.\n");
				successTXFlag = 0;
				nextState = S_Idle;
			}
			else if(newDataFlag) {
				printf("Max retries.\n");
				maxRTFlag = 0;
				nextState = S_Idle;
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
void nrfSendMessage(uint8_t *str, uint8_t str_len, uint8_t *pipe)
{
	PORTC.OUTSET = PIN0_bm;
	nrfStopListening();
	nrfOpenWritingPipe(pipe);
	delay_us(130);
	
	nrfStartWrite(str, str_len, NRF_W_TX_PAYLOAD_NO_ACK);
	_delay_us(300);
	
	for (uint8_t i = 0; i < str_len; i++)
	{
		_delay_us(40);
	}
	nrfWriteRegister(REG_STATUS, NRF_STATUS_TX_DS_bm|NRF_STATUS_MAX_RT_bm);
	
	nrfStartListening();
	delay_us(130);
	PORTC.OUTCLR = PIN0_bm;
	
}

void SendRouting( void )
{
	uint8_t *str = getRoutingString();
	
	nrfSendMessage(str, str[2], broadcast_pipe);
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
	
	DB_MSG("\n----Debug mode enabled----\n\n");
	printf_DeviceSerial(device_serial,11);

	_delay_ms(200);
}

/* This function will be called when state equals S_GotMail.
	It will parse the message to determine what kind of message 
	it is, and what to do with it. UMT means Unknown Message Type */
void parseIncomingData( void )
{
	switch(packet[0])
	{
		case BROADCAST:
		 	printf("0x%02X %d %s\n", packet[0], packet[1], packet + 2);
			break;
		case RHDR:
			addNeighbor(packet[1]);
			printf_Routing(packet, packet[2]);
			FillRoutingTable(packet, packet[2]);
			break;
		case DHDR:
		break;
		case BCREPLY:
		break;
		default:
		 	printf("UMT: ");
			//printf_hex(packet, sizeof(packet));
			printf_bin(packet, sizeof(packet));
			break;
	}
}

const uint8_t getID(){
	if(FB) return 51;
	else if(RB) return 52;
	else if(SB) return 53;
	else if(MF) return 83;
	else if(JG) return 77;
	else return 00;
}

void init_nrf(const uint8_t pvtID){
	nrfspiInit();
	nrfBegin();

	nrfSetRetries(NRF_SETUP_ARD_1000US_gc,	NRF_SETUP_ARC_10RETRANSMIT_gc);
	nrfSetPALevel(NRF_RF_SETUP_PWR_18DBM_gc);
	nrfSetDataRate(NRF_RF_SETUP_RF_DR_250K_gc);
	nrfSetCRCLength(NRF_CONFIG_CRC_16_gc);
	nrfSetChannel(channel);
	nrfSetAutoAck(1);
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