/*!
 *  \file    stream.c
 *  \author  Wim Dolman (<a href="email:w.e.dolman@hva.nl">w.e.dolman@hva.nl</a>)
 *  \date    03-10-2016
 *  \version 1.4
 *
 *  \brief   Stream for the communcation mode of HvA-Xmegaboard
 *
 *  \details Makes a standard stream for the HvA-Xmegaboard. It is build on the wrapper.
 *           With these you can use <code>printf</code> and <code>scanf</code> to write
 *           to and read from the UART F0. The baud rate is 115200 and the protocol is 8N1.
 *
 *           Version 1.3 :  Modified uart_fputc changed. With a full TX-buffer it doesn't
 *           skips characters anymore. It now waits until there is free space in the buffer.
 *
 *           Version 1.4 :  The baud rate is changed toe 115200 
 */

#include <avr/io.h>
#include "stream.h"

#define ENABLE_UART_F0   1         //!< UART0 from PORTF
#define ENABLE_UART_E0   1         //!< UART0 from PORTE
#include "uart.h"

/*! \brief Write a byte to the circular transmit buffer.
 *         This function is necessary for a standard stream
 *
 *  \param  c       character  to be written
 *  \param  stream  file pointer
 *
 *  \return always zero
 */
int uart_fputc(char c, FILE *stream)
{
  // uartF0
  while( ! USART_TXBuffer_FreeSpace(&uartF0) );

	//Removed for 'Sensornetwerken Ontwerp'
  //if (c == '\n') uart_putc(&uartF0, '\r');
  uart_putc(&uartF0, c);

  // uartE0
  while( ! USART_TXBuffer_FreeSpace(&uartE0) );
	
	//Removed for 'Sensornetwerken Ontwerp'
  //if (c == '\n') uart_putc(&uartE0, '\r');
  uart_putc(&uartE0, c);

  return 0;
}


FILE uart_stdinout_usb_and_io = FDEV_SETUP_STREAM(uart_fputc, NULL, _FDEV_SETUP_WRITE);  //!< FILE structure for standard streams

/*! \brief Initialize a standard in/out stream
 *         It uses USART0 of port F, the baud rate is BAUD,
 *         doubleclockspeed is CLK2X and
 *         the interruptlevel is low
 *         The global interrupt is not yet set.
 *
 *  \param  fcpu  the clock frequency
 *
 *  \return void
 */
void init_stream(uint32_t fcpu)
{
  init_uart(&uartF0, &USARTF0, fcpu, BAUD, CLK2X);
  init_uart(&uartE0, &USARTE0, fcpu, BAUD, CLK2X);
  
  stdout = &uart_stdinout_usb_and_io;

  PMIC.CTRL |= PMIC_LOLVLEN_bm;           // Low level interrupt
}

