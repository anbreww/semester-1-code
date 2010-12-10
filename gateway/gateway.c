#define F_CPU 2000000UL

// *** General AVR Libraries ***
#include <avr/io.h>
#include <util/delay.h>

// *** XMEGA Libraries ***
#include "../xmega_libs/clksys_driver.h"
#include "../xmega_libs/usart_driver.h"

// *** Project specific libraries ***
#include "../project_libs/defines.h"
#include "../project_libs/hardware.h"
#include "../project_libs/xbee.h"
#include "../project_libs/tbar.h"
#include "../project_libs/debug.h"




// rf data packet
char packet[255];



int main(void)
{
	CLKSYS_Prescalers_Config( CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc );


	// CLKOUT on PC7
	PORTC.DIRSET = (1<<7);
	PORTCFG.CLKEVOUT = PORTCFG_CLKOUT_PC7_gc;


	tbar_init();

	xb_init();

	PORTB.DIRSET = 1; // set PB0 as output

	DEBUG_PORT.DIRSET = DEBUG_RED + DEBUG_GREEN;


	while(1)
	{
		//uint8_t i;
		// wait for incoming transmission from Zigbee
		// TODO : make this into interrupt-driven stuffs

		_delay_ms(600);

		// signal that we're ready
		debug_blink(DEBUG_GREEN);

		while(1)
		{
			//debug_blink(DEBUG_GREEN);
			char byte = 0;
			XB_GetChar(byte);
			if(byte == 0x7e)
			{
				uint8_t length = xb_decode_packet(packet);

				if(length>0)
				{
					uint8_t i;
					for(i=0;i<length; i++)
					{
						TB_PutChar(packet[i]);
					}
					debug_blink(DEBUG_GREEN);
				}
				else
					debug_blink(DEBUG_RED);
			}
		}

		// signal that we've started receiving
		debug_blink(DEBUG_RED);

		


		// decode packet

		// send data out T port
	/*
		for (i=0; i<sizeof(Coord.name)/sizeof(char); i++)
		{
			while( (USARTT.STATUS & USART_DREIF_bm) == 0);
			USARTT.DATA = Coord.name[i];
			
		}
		*/

	/*	xb_send_array_new(blah, sizeof(blah)/sizeof(char), &Coord);
		xb_send_array_new(blah, sizeof(blah)/sizeof(char), &Gizmo1);*/
		// flash green led
		
		//USARTT_PORT.OUTTGL = (1<<USARTT_TX);
	}
}



