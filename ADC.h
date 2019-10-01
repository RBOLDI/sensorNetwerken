/*
 * ALDC.h
 *
 * Created: 1-10-2019 09:27:08
 *  Author: Rowan
 */ 


#ifndef ALDC_H_
#define ALDC_H_

#define F0_CLK2x		0
void init_adc(void);
void ADC_timer(void);
uint8_t ADC_sample(uint8_t takeSample);
uint8_t sampleData[2];
volatile uint16_t res;


#endif /* ALDC_H_ */