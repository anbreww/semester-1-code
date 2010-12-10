#include <avr/io.h>

#include "hardware.h"

#include "tbar.h"

#include "../xmega_libs/usart_driver.h"


void tbar_init()
{
	/* Enable T-Bar USART */
	USARTT_PORT.DIRSET = PIN7_bm;


	USART_Format_Set(&USARTT, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
	USART_Baudrate_Set(&USARTT, 144, -6);
	USART_Tx_Enable(&USARTT);
}
