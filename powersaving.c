/*
 * powersaving.c
 *
 * Created: 09/09/2019 16:24:45
 *  Author: Mike
 */ 

void init_lowpower(){
	PR.PRGEN   |= PR_USB_bm | PR_AES_bm | PR_RTC_bm | PR_EVSYS_bm | PR_DMA_bm;
	PR.PRPA	   |= PR_DAC_bm | PR_ADC_bm | PR_AC_bm;
	PR.PRPB	   |= PR_DAC_bm | PR_ADC_bm | PR_AC_bm;
	PR.PRPC    |= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
	PR.PRPD    |= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
	PR.PRPE    |= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
	PR.PRPF    |= PR_TWI_bm | PR_USART1_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
}

void idle(){
	SLEEP.CTRL |= SLEEP_MODE_IDLE | SLEEP_SEN_bm;
}