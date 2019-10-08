/*
 * ALDC.h
 *
 * Created: 1-10-2019 09:27:08
 *  Author: Rowan
 */ 


#ifndef ALDC_H_
#define ALDC_H_

void init_adc(void);
void ADC_timer(void);
uint8_t ADC_sample(void);
uint8_t sampleData[2];
volatile uint8_t sampleFlag; 
volatile uint16_t res;
volatile uint8_t ADCCounter;

#endif /* ALDC_H_ */