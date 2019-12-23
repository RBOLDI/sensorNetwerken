/*
 * ADC.c
 *
 * Created: 1-10-2019 09:26:56
 *  Author: Rowan
 */ 
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stddef.h>             // definition of offsetof
#include <avr/interrupt.h>
#include <stdio.h>
#include "ADC.h"

#define TIMERCOUNTSBETWEENSAMPLES 6 // Count to this amount between new sample. Every count is 5 seconds.

//Function prototypes
uint16_t readCalibrationWord(uint8_t index);

//Variable for res.
volatile uint16_t res = 0;
volatile uint8_t sampleFlag = 0;
//Sample
uint8_t sampleData[2];

//-------------------------------------------------
//					ADC FUNCTIONS				 --
//-------------------------------------------------
uint16_t readCalibrationWord(uint8_t index){
	uint16_t result;

	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	result  = pgm_read_word(index);
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;

	return result;
}

//Initialization of the xMega ADC
void init_adc(void){
	ADCA.CAL = readCalibrationWord( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0));
	PORTA.DIRCLR     = PIN2_bm;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc;		// PA2 as + of CH0 Internal GND as - of CH0.
	ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_SINGLEENDED_gc;	// Diff no offset
	ADCA.REFCTRL     = ADC_REFSEL_INTVCC_gc;

	ADCA.CTRLB       = ADC_RESOLUTION_12BIT_gc;
	ADCA.PRESCALER   = ADC_PRESCALER_DIV16_gc;
	ADCA.CTRLA       = ADC_ENABLE_bm;
}


uint16_t read_adc(void)
{
	ADCA.CH0.CTRL |= ADC_CH_START_bm;                    // start ADC conversion
	while ( !(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm) ) ;    // wait until it's ready
	res = ADCA.CH0.RES;
	ADCA.CH0.INTFLAGS |= ADC_CH_CHIF_bm;                 // reset interrupt flag

	return res;                                          // return measured value
}

//Take sample function
//Returns 0 if new sample is taken, 1 when there is no new sample.
uint8_t ADC_takesample(void){
	if(sampleFlag)
	{
		sampleFlag = 0;
		res = read_adc();
		sampleData[0] = res >> 8;		// Take 8 MSB's as first data byte
		sampleData[1] = res & 0x00FF;	// Take 8 LSB's as second data byte
		return 1;
	} 
	
	return 0;
}

void ADC_timer(void){
	if (++ADCCounter % TIMERCOUNTSBETWEENSAMPLES == 0) sampleFlag = 1;
}