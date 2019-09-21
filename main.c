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
#include "nrf24L01.h"
#include "nrf24spiXM2.h"
#include "stream.h"
#include "uart.h"
#include "KeyboardCodes.h"
#include "powersaving.h"
#include "routingtable.h"
#include "messages.h"
#include "debug_opts.h" 

#define		FB !(PORTD.IN & PIN1_bm)
#define		RB !(PORTD.IN & PIN2_bm)
#define		SB !(PORTD.IN & PIN3_bm)
#define		MF !(PORTD.IN & PIN4_bm)
#define		JG !(PORTD.IN & PIN5_bm)

#define		BROADCAST	0x01
#define		RRTABLE		0x02
#define		RXPTABLE	0x03
#define		BCREPLY		0x04

#define		MESSAGE_BUFFER_SIZE 320

//Function prototypes
const uint8_t getID();
uint8_t writeMessage();

uint8_t newDataFlag = 0;
uint8_t sendDataFlag = 0;
uint8_t newKeyboardData = 0;
uint8_t newBroadcastFlag = 0;
uint8_t bigMessageFlag = 0;

uint8_t MYID;

tMessage mBrCast;

enum states {
	S_Boot,
	S_Broadcast,
	S_Idle,
	S_GotMail,
	S_Send,
	S_Long
};

enum states currentState, nextState = S_Boot;

int pointer = 0;
char charBuffer[MAX_MESSAGE_SIZE] = {0};


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

ISR(PORTD_INT0_vect)
{
	PORTF.OUTTGL = PIN1_bm;
	newBroadcastFlag = 1;
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

/* This function will allow the microcontroller to send multiple packets
of 32 bytes each. It uses NO_ACK setting to prevent the mcu from getting
stuck when receiving acknowledge messages. NO_ACK mode is significantly
faster than ACK mode.
*/
void nrfSendLongMessage(uint8_t *str, uint8_t str_len, uint8_t *pipe)
{
	PORTC.OUTSET = PIN0_bm;
	nrfStopListening();
	
	nrfOpenWritingPipe(pipe);

	printf_hex(str,str_len);
	
	while(str_len>32)
	{
		nrfStartWrite(str, 32, NRF_W_TX_PAYLOAD_NO_ACK);

		str += 32;
		str_len -= 32;
		
	}
	nrfStartWrite(str, str_len, NRF_W_TX_PAYLOAD_NO_ACK);
	
	nrfStartListening();
	
	PORTC.OUTCLR = PIN0_bm;
}

bool bigMessage(uint8_t * arr)
{
	uint8_t len = arr[2];
	if (len > 32) return true;
	else return false;
}

void makeBuffer(tMessage *message)
{
	message->msgBuffer = (uint8_t *)malloc(MESSAGE_BUFFER_SIZE*sizeof(uint8_t));
}

void resetBuffer(tMessage *message){
	memset(message->msgBuffer, 0, MESSAGE_BUFFER_SIZE*sizeof(uint8_t));
	message->buffPos = 0;
	message->len = 0;
}

uint8_t parseLong(uint8_t *arr, tMessage *msg, uint8_t flag)
{
	PORTF.OUTTGL = PIN0_bm;
	// The return value rc indicates if the end of the message is reached.
	int rc = 0;
	switch (flag)
	{
		/*First time function is used, the message length is stored in tMessage
		 and the first packet is stored in the message buffer.*/
		case 0:
			msg->len = arr[2];
			printf("len=%d, buffPosF0=%d\n", msg->len, msg->buffPos);
			memcpy(msg->msgBuffer, arr, sizeof(packet));
			msg->buffPos += 32;
			rc = 0;
			break;
		/*The rest of the packets will be parsed from here. After the long message
		is printed, the (long message)flag will be set back to 0. */
		case 1:
			if(msg->buffPos < (msg->len - (msg->len % 32))){
				msg->msgBuffer += msg->buffPos;
				memcpy(msg->msgBuffer, arr, sizeof(packet));
				msg->buffPos += 32;
				printf("buffPosF0=%d\n", msg->buffPos);
				rc = 0;
				
			}else{
				msg->msgBuffer += msg->buffPos;
				memcpy(msg->msgBuffer, arr, (msg->len % 32)*sizeof(uint8_t));
				printf("buffPosF1=%d\n", msg->buffPos);
				msg->msgBuffer -= (msg->buffPos);
				printf("0x%02X %d %d %s\n", msg->msgBuffer[0], msg->msgBuffer[1], msg->msgBuffer[2], msg->msgBuffer + 3);
				rc = 1;
			}
			break;
	}	
	return rc;
}
 
void broadcast(void)
{
	uint8_t *str = GetRoutingString(MYID);

	nrfSendLongMessage(str, str[2], broadcast_pipe);
}

/* This function will be called when state equals S_Boot.
	It will run all the initializations of the Xmega. Including I/O,
	setting MYID, UART stream and nRF */
void bootFunction(void)
{
	makeBuffer(&mBrCast);
	
	init_io();
	
	MYID = getID();
	memmove(initials, get_user_initials(MYID), NUMBER_OF_PREFIX_BYTES);

	init_stream(F_CPU);

	init_nrf(MYID);
	
	init_routingtable();

	_delay_ms(200);

}


/* This function will be called when state equals S_GotMail.
	It will parse the message to determine what kind of message 
	it is, and what to do with it. UMT means Unknown Message Type */
void parseIncomingData(void)
{
	PORTF.OUTTGL = PIN0_bm;

	switch(packet[0])
	{
		case BROADCAST:
		 	printf("0x%02X %d %s\n", packet[0], packet[1], packet + 2);
			break;
		case RRTABLE:
			printf("0x%02X %d %d %s\n", packet[0], packet[1], packet[2], packet + 3);
			break;
		case RXPTABLE:
		case BCREPLY:
		default:
		 	printf("UMT: %s\n", packet);
			break;
	}
}

int main(void)
{
    while (1) 
    {
		switch(currentState) {
			case S_Boot:
				bootFunction();
				nextState = S_Broadcast;
				break;
			case S_Broadcast:
				broadcast();
				nextState = S_Idle;
				break;
			case S_GotMail:
				if(bigMessage(packet)){
					parseLong(packet, &mBrCast, bigMessageFlag);
					bigMessageFlag = 1;
					nextState = S_Long;
				}
				else{
					parseIncomingData();
					nextState = S_Idle;
				}
				break;
			case S_Long:
				if(parseLong(packet, &mBrCast, bigMessageFlag)){
					printf_hex(mBrCast.msgBuffer,mBrCast.len);
					resetBuffer(&mBrCast);
					bigMessageFlag = 0;
				}else{};
				nextState = S_Idle;
				break;
			case S_Idle:
				if(newBroadcastFlag) {
					newBroadcastFlag = 0;
					nextState = S_Broadcast;
				}
				else if(newDataFlag && bigMessageFlag){
					newDataFlag = 0;
					nextState = S_Long;
				}
				else if(newDataFlag) {
					newDataFlag = 0;
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
		
		if(newKeyboardData)
		{
			newKeyboardData = 0;
			writeMessage(&message);
		}

		currentState = nextState;
    }
}

uint8_t writeMessage(char* msg){
	char c = uart_fgetc(&uart_stdinout);
	
	if (c == ENTER)
	{
		charBuffer[pointer] = '\0';
		
		strcpy(msg,charBuffer);
		sendMessage(51);

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

