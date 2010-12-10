#ifndef	XBEE_H
#define XBEE_H


// escaped characters ( in API mode 2)
#define START_BYTE	0x7E
#define ESCAPE		0x0D
#define XON			0x11
#define XOFF		0x13

#define XB_START_BYTE		0x7E
#define XB_SEND_RQ			0x10
#define XB_EXPLICIT_SEND	0x11


#define	SEND_REQUEST	0x10
#define EXPLICIT_SEND	0x11

#define BROADCAST_ADDR	0xFFFE


typedef struct XBEE_struct {
	uint8_t addr64[8];
	uint8_t addr16[2];
	char name[20];
} XBee;

#define XB_GetChar(byte)	while(!(USART_IsRXComplete(&USARTX))); \
							byte = USART_GetChar(&USARTX);
#define TB_PutChar(byte)	while( (USARTT.STATUS & USART_DREIF_bm) == 0); \
							USART_PutChar(&USARTT, byte);



void xb_init();
void xb_send_frame(USART_t * _usart);
void xb_send_array(char * _array, uint8_t _length, XBee * xb);

// cleaner implementation of this function
void xb_send_array_new(char * _array, uint8_t _length, XBee * xb);

uint8_t xb_decode_packet(char * _packet);


// set 16 and 64 bit addresses. set addr16 to 0 or 0xfffe to use only 64 bit address.
void xb_set_addr(XBee * _xb, uint8_t * _addr64, uint16_t _addr16);

#endif
