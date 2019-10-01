/*
 * powersaving.c
 *
 * Created: 09/09/2019 16:24:45
 *  Author: Mike
 */ 
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdlib.h>
#include "clksys_driver.h"
#include "powersaving.h"

void init_lowpower(void)
{
	PR.PRGEN   |= PR_USB_bm | PR_AES_bm | PR_RTC_bm | /*PR_EVSYS_bm |*/ PR_DMA_bm;
	//PR.PRPA	   |= PR_DAC_bm | PR_ADC_bm | PR_AC_bm;
	PR.PRPB	   |= PR_DAC_bm | PR_ADC_bm | PR_AC_bm;
	PR.PRPC    |= PR_TWI_bm | PR_USART1_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;					//UART C0 aan voor USB
	PR.PRPD    |= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm; //| PR_TC0_bm;
	PR.PRPE    |= PR_TWI_bm | PR_USART1_bm | PR_USART0_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm;
	PR.PRPF    |= PR_TWI_bm | PR_USART1_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm |PR_TC0_bm;					//UART F0 aan voor NRF
}

void idle(void){
	SLEEP.CTRL |= SLEEP_MODE_IDLE | SLEEP_SEN_bm;
}

void InitClocks(void)
{
	CLKSYS_Enable( OSC_RC2MEN_bm );
	do {} while ( CLKSYS_IsReady( OSC_RC2MRDY_bm ) == 0 );
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_RC2M_gc);
	CLKSYS_Disable( OSC_RC2MEN_bm );	
}
	
void init_io(void)
{
	PORTF.DIRSET = PIN0_bm | PIN1_bm;
	PORTC.DIRSET = PIN0_bm;
	
	//Set ID selector pins
	PORTD.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
	
	PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN3CTRL = PORT_OPC_PULLUP_gc;
	PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc;	
	PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc;
	
	//Define unused pins to save energy
	PORTA.DIRCLR = PIN0_bm | PIN1_bm | /*PIN2_bm | PIN3_bm | */PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
	PORTB.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
	PORTD.DIRCLR = PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
	PORTE.DIRCLR = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;
	
	//Intern Pull-ups
	PORTA.PIN0CTRL = PORT_OPC_PULLUP_gc;	PORTA.PIN1CTRL = PORT_OPC_PULLUP_gc;
	//PORTA.PIN2CTRL = PORT_OPC_PULLUP_gc;	PORTA.PIN3CTRL = PORT_OPC_PULLUP_gc;
	PORTA.PIN4CTRL = PORT_OPC_PULLUP_gc;	PORTA.PIN5CTRL = PORT_OPC_PULLUP_gc;
	PORTA.PIN6CTRL = PORT_OPC_PULLUP_gc;	PORTA.PIN7CTRL = PORT_OPC_PULLUP_gc;
	
	PORTB.PIN0CTRL = PORT_OPC_PULLUP_gc;	PORTB.PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTB.PIN2CTRL = PORT_OPC_PULLUP_gc;	PORTB.PIN3CTRL = PORT_OPC_PULLUP_gc;
	PORTB.PIN4CTRL = PORT_OPC_PULLUP_gc;	PORTB.PIN5CTRL = PORT_OPC_PULLUP_gc;
	PORTB.PIN6CTRL = PORT_OPC_PULLUP_gc;	PORTB.PIN7CTRL = PORT_OPC_PULLUP_gc;
	
	PORTD.PIN6CTRL = PORT_OPC_PULLUP_gc;	PORTD.PIN7CTRL = PORT_OPC_PULLUP_gc;
	
	PORTE.PIN0CTRL = PORT_OPC_PULLUP_gc;	PORTE.PIN1CTRL = PORT_OPC_PULLUP_gc;
	PORTE.PIN4CTRL = PORT_OPC_PULLUP_gc;	PORTE.PIN5CTRL = PORT_OPC_PULLUP_gc;

	//Button
	PORTD.INT0MASK = PIN0_bm;
	PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
	PORTD.INTCTRL  = PORT_INT0LVL_LO_gc;
	
	//Timer
	TCE0.CTRLB = TC_WGMODE_NORMAL_gc;
	TCE0.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCE0.INTCTRLA = TC_OVFINTLVL_LO_gc;	
	TCE0.PER = 9765;						// PER = (5 * FCPU)/PRESCALER - 1 // t = 5,000192 s
	
}
