#include <avr/io.h>

#include "xbee.h"
#include "../xmega_libs/usart_driver.h"
#include "hardware.h"

void xb_init(void)
{
	// TX as Output
	USARTX_PORT.DIRSET = (1<<USARTX_TX);
	// RX as Input
	USARTX_PORT.DIRCLR = (1<<USARTX_RX);


	USART_Format_Set(&USARTX, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
	USART_Baudrate_Set(&USARTX, 144, -6);

	/* Enable XBee USART */
	USART_Tx_Enable(&USARTX);
	USART_Rx_Enable(&USARTX);

	XB_SLEEP_RQ_PORT.DIRSET = (1<<XB_SLEEP_RQ);

}

char xb_frame[] = {	
					START_BYTE, 		// start delimiter
					0x00, 0x0F,	// message size (MSB, LSB)
					0x10,		// message type (10 : send rq, 11 : explicit send)
					0x01,		// frame ID
					0x00, 0x00, 0x00, 0x00,	// 64-bit address (part1)
					0x00, 0x00, 0x00, 0x00, // 64-bit address (part2)
					0xFF, 0xFE,	// 16 bit addr (set to FFFE when using 64 bit addr)
					0x00,		// broadcast radius 1-15 (0 = max hops)
					0x00,		// options
					0x31,		// payload
					0xC0		// checksum
				};


void xb_send_frame(USART_t * _usart)
{
	static uint8_t frame_id  = 0;
	uint8_t i = 0;
	uint8_t cksum = 0;
	for(i=0; i<18; i++)
	{
		char byte = xb_frame[i];

		if(i == 4)
			byte = frame_id;
		else if (i == 17)
			byte = frame_id;
		if(i>=3)
			cksum += byte;

		while( (_usart->STATUS & USART_DREIF_bm) == 0);
		_usart->DATA = byte;
	}
	while( (_usart->STATUS & USART_DREIF_bm) == 0);
	_usart->DATA = (0xFF - cksum);
	frame_id++;
}

#define OVERHEAD 14 // non-payload bytes
void xb_send_array(char * _array, uint8_t _length, XBee * xb)
{
	// blah
	USART_t * _usart = &USARTX;
	static uint8_t frame_id  = 0;

	uint8_t i = 0;
	uint8_t cksum = 0;

	uint16_t message_size;
	message_size = _length + OVERHEAD;

	
	for(i=0; i<17 + _length; i++)
	{
		char byte = xb_frame[i];

		if(i == 1) // MSB
			byte = (message_size >> 8) & 0xFF;
		else if(i == 2) // LSB
			byte = (message_size & 0xFF);
		else if(i == 4)
			byte = frame_id;
		else if (i > 16)
			byte = _array[i-17];


		if(i>=3)
			cksum += byte;

		while( (_usart->STATUS & USART_DREIF_bm) == 0);
		_usart->DATA = byte;
	}
	while( (_usart->STATUS & USART_DREIF_bm) == 0);
	_usart->DATA = (0xFF - cksum);
	frame_id++;
}

void uart_putchar(USART_t * _usart, char _c)
{
	while( (_usart->STATUS & USART_DREIF_bm) == 0);
		_usart->DATA = _c;
}

void xb_send_array_new(char * _array, uint8_t _length, XBee * xb)
{
	// Communication goes through USARTX as defined in hardware.h

	// Keep track of frame counter to keep track of transmissions
	static uint8_t frame_id  = 0;

	uint16_t i = 0;


	uint16_t message_size = _length + OVERHEAD;
	// Total message size = message_size + 2 (size) + 1 (checksum)

	uint8_t cksum = 0;

	// See XBee datasheet p.108 for message construction

	for(i=0; i<message_size+3; i++)
	{
		char byte = 0;
		// start delimiter
		if(i == 0)
			byte = XB_START_BYTE;
		// message size (MSB, LSB) (excluding size and cksum)
		else if(i == 1)
			byte = (message_size >> 8) & 0xFF;
		else if(i == 2)
			byte = (message_size & 0xFF);
		// message type (10 : send rq, 11 : explicit send)
		else if(i == 3)
			byte = XB_SEND_RQ;
		// frame ID
		else if(i == 4)
			byte = frame_id;
		// 64-bit address
		else if(i >= 5 && i <= 12)
			byte = xb->addr64[i-5];
		// 16 bit addr (set to FFFE when using 64 bit addr)
		else if(i >= 13 && i <= 14)
			byte = xb->addr16[i-13];
		// broadcast radius 1-15 (0 = max hops)
		else if(i == 15)
			byte = 0;
		// options
		else if(i == 16)
			byte = 0;
		// payload
		else if(i >= 17)
			byte = _array[i-17];

		// checksum calculation
		if( i >= 3)
			cksum += byte;

		uart_putchar(&USARTX, byte);
	}
	
	// final char : checksum
	uart_putchar(&USARTX, 0xFF-cksum);

	// increment before sending next frame
	frame_id++;
}

// returns length of packet, 0 in case of error
uint8_t xb_decode_packet(char * _packet)
{
	// we've caught 0x7e, now unpack the frame
	char byte = 0;
	char cksum = 0;

	// MSB
	uint16_t message_size = 0;
	XB_GetChar(byte);
	message_size = ((uint16_t) byte)<<8;

	// LSB
	XB_GetChar(byte);
	message_size += byte;

	// Message type
	XB_GetChar(byte);
	message_size--;
	cksum += byte;
	if (byte == 0x90)
		; // explicit rx
	else
		return 0; // ERROR

	// 64-bit address of sender
	uint8_t i;
	for(i=0; i<8;i++)
	{
		XB_GetChar(byte);
		message_size--;
		cksum += byte;
	}

	// 16-bit address of sender
	for(i=0; i<2;i++)
	{
		XB_GetChar(byte);
		message_size--;
		cksum += byte;
	}

	// mode (01)
	XB_GetChar(byte);
	message_size--;
	cksum += byte;

	// data
	for(i=0; i<message_size; i++)
	{
		XB_GetChar(byte);
		_packet[i] = byte;
		cksum += byte;
	}

	// checksum
	XB_GetChar(byte);

	if (0xff - cksum == byte)
		return message_size;
	else
		return 0; // ERROR
	//TB_PutChar(byte);


}



void xb_set_addr(XBee * _xb, uint8_t * _addr64, uint16_t _addr16)
{
	uint8_t i;
	for(i=0; i<8; i++)
		_xb->addr64[i] = _addr64[i];
	if(_addr16 == 0)
		_addr16 = 0xfffe;
	_xb->addr16[0] = (_addr16>>8)&0xff;
	_xb->addr16[1] = (_addr16&0xFF);
}

