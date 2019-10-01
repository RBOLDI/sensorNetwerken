/*
 * ALDC.h
 *
 * Created: 1-10-2019 09:27:08
 *  Author: Rowan
 */ 


#ifndef ALDC_H_
#define ALDC_H_

void init_adc(void);
uint8_t ADC_sample(uint8_t takeSample);
uint8_t sampleData[2];
uint16_t res;


#endif /* ALDC_H_ */