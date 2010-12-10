#include <avr/io.h>

#include "xbee.h"
#include "libs/usart_driver.h"



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

struct xbee {
	uint8_t addr64[8];
	uint8_t addr16[2];
	char name[20];
}

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

#define OVERHEAD 14
void xb_send_array(char * _array, uint8_t _length, uint16_t _destination, USART_t * _usart)
{
	// blah
	static uint8_t frame_id  = 0;
	uint8_t i = 0;
	uint8_t cksum = 0;
	uint16_t message_size = OVERHEAD;
	message_size += _length;

	
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


