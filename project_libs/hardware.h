#ifndef F_CPU
#define F_CPU 2000000UL
#endif

// Ambient light sensor on ADC
#define AMBIENT_PORT		PORTA
#define AMBIENT_PIN			7
#define AMBIENT_ENABLE		6
#define AMBIENT_ENABLE_bm	PIN6_bm

// Debug PORT
#define DEBUG_PORT			PORTF
#define DEBUG0				0
#define DEBUG1				1
#define DEBUG2				2
#define DEBUG3				3
#define DEBUG4				4
#define DEBUG5				5

#define DEBUG_GREEN			2
#define DEBUG_RED			1

// XBee USART
#define USARTX				USARTE0
#define USARTX_PORT			PORTE
#define USARTX_RX			2
#define USARTX_TX			3
#define USARTX_RXC_vect		USARTE0_RXC_vect

// XBee Connections
#define XB_RESET_PORT		PORTD
#define XB_RESET			4

#define XB_SLEEP_PORT		PORTE
#define XB_SLEEP			0

#define XB_SLEEP_RQ			1
#define XB_SLEEP_RQ_PORT	PORTE

// Barras Module USART
#define USARTT				USARTD1
#define USARTT_PORT			PORTD
#define USARTT_RX			6
#define USARTT_TX			7

// ----- Charger connections ------

#define CHARGER_PORT		PORTD

#define CHARGER_ENABLE		1 // on port D
#define CHARGER_RX			2
#define CHARGER_TX			3
#define CHARGER_USART		USARTD0

// ***** Sensor I/O *****
#define VCC3_ENABLE			0 // on port D

// 1  VCC	20 3V3_SW
// 2  SDA	19 PB0
// 3  SCL	18 PB1
// 4  RXD	17 DAC0
// 5  TXD	16 DAC1
// 6  SS	15 ADC3
// 7  MOSI	14 ADC2
// 8  MISO	13 ADC1
// 9  SCK	12 ADC0
// 10 GND	11 GND




