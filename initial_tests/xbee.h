#ifndef	XBEE_H
#define XBEE_H


// escaped characters ( in API mode 2)
#define START_BYTE	0x7E
#define ESCAPE		0x0D
#define XON			0x11
#define XOFF		0x13


#define	SEND_REQUEST	0x10
#define EXPLICIT_SEND	0x11

#define BROADCAST_ADDR	0xFFFE



void xb_send_frame(USART_t * _usart);
void xb_send_array(char * _array, uint8_t _length, uint16_t _destination, USART_t * _usart);

;

#endif
