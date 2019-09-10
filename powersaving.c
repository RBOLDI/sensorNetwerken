/*
 * powersaving.c
 *
 * Created: 09/09/2019 16:24:45
 *  Author: Mike
 */ 

void init_lowpower(void){
	PR.PRGEN   |= PR_USB_bm | PR_AES_bm | PR_RTC_bm | PR_EVSYS_bm | PR_DMA_bm;
	PR.PRPA	   |= PR_DAC_bm | PR_ADC_bm | PR_AC_bm;
	PR.PRPB	   |= PR_DAC_bm | PR_ADC_bm | PR_AC_bm;
	PR.PRPC    |= PR_TWI_bm | PR_USART1_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;					//UART C0 aan voor USB
	PR.PRPD    |= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
	PR.PRPE    |= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
	PR.PRPF    |= PR_TWI_bm | PR_USART1_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;					//UART F0 aan voor NRF
}

void idle(void){
	SLEEP.CTRL |= SLEEP_MODE_EXT_STANDBY | SLEEP_SEN_bm;
}

static void InitClocks(void) {
	CLKSYS_Enable( OSC_RC32MEN_bm );
	do {} while ( CLKSYS_IsReady( OSC_RC32MRDY_bm ) == 0 );
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_RC32M_gc );
	CLKSYS_Disable( OSC_RC2MEN_bm );	
	}