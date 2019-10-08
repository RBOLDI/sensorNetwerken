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
	PORTA.DIRCLR     = PIN2_bm|PIN3_bm;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc|		// PA2 as + of CH0.
					   ADC_CH_MUXNEG_PIN3_gc;		// Internal GND as - of CH0.
	ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;	// Diff no offset
	ADCA.CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;			// low lvl interrupt for reading.
	ADCA.REFCTRL     = ADC_REFSEL_INTVCC_gc;
	
	ADCA.CTRLB       = ADC_RESOLUTION_12BIT_gc |	// 12 bits conversion
					   ADC_CONMODE_bm | 
					   !ADC_FREERUN_bm;				// no free run
	ADCA.PRESCALER   = ADC_PRESCALER_DIV16_gc;
	ADCA.CTRLA       = ADC_ENABLE_bm;
	ADCA.EVCTRL      = ADC_SWEEP_0_gc |				// sweep CH0
					   ADC_EVSEL_0123_gc |			// select event CH0,1,2,3
					   ADC_EVACT_CH0_gc;			// event triggers ADC CH0
	EVSYS.CH0MUX     = EVSYS_CHMUX_TCD0_OVF_gc;		// event overflow timer E0
}

//Take sample function
//Returns 0 if new sample is taken, 1 when there is no new sample.
uint8_t ADC_sample(void){
	uint16_t copyData;
	if(sampleFlag){
		sampleFlag = 0;
		copyData = res & 0xFF00; copyData = copyData >> 8;
		sampleData[0] = copyData;
		copyData = res & 0x00FF; 
		sampleData[1] = copyData;
		return 1;
	} 
	else
		return 0;
}

void ADC_timer(void){
	TCD0.PER      = 31249;						// Tper =  256 * (31249 +1) / 2M = 4s
	TCD0.CTRLA    = TC_CLKSEL_DIV256_gc;        // Prescaling 8
	TCD0.CTRLB    = TC_WGMODE_NORMAL_gc;        // Normal mode
	TCD0.INTCTRLA = TC_OVFINTLVL_OFF_gc;        // Interrupt overflow off
}

ISR(ADCA_CH0_vect){
	res = ADCA.CH0.RES;
	sampleFlag = 1;
}