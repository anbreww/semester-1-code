// general purpose libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// i/o libs
#include <stdio.h>
#include <string.h>
#include <math.h>

// load board configuration
#include "../project_libs/hardware.h"

// AVR-made drivers
#include "../xmega_libs/clksys_driver.h"
#include "../xmega_libs/usart_driver.h"

// Project-specific functions
#include "../project_libs/xbee.h"
#include "../project_libs/debug.h"
#include "builtin_sensors.h"


#define RX_BUFFER_SIZE	50
#define TX_BUFFER_SIZE	200


/*
 *	Function prototypes
 */
void init_clock(void);

uint8_t node1_64[] =  { 0x00, 0x13, 0xa2, 0x00, 0x40, 0x64, 0xef, 0xcc };
const uint8_t node2_64[] =  { 0x00, 0x13, 0xa2, 0x00, 0x40, 0x64, 0xef, 0xeb };
const uint8_t gizmo0_64[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const uint8_t gizmo1_64[] = { 0x00, 0x13, 0xa2, 0x00, 0x40, 0x34, 0x1d, 0x05 };

char payload[TX_BUFFER_SIZE];
uint8_t	payload_length = sizeof(payload)/sizeof(char);
char rx_buffer[RX_BUFFER_SIZE];

#define xb_wake()	XB_SLEEP_RQ_PORT.OUTCLR = (1<<XB_SLEEP_RQ);
#define xb_sleep()	XB_SLEEP_RQ_PORT.OUTSET = (1<<XB_SLEEP_RQ);
#define xb_is_awake()	((XB_SLEEP_PORT.IN & (1<<XB_SLEEP)) >> XB_SLEEP)

void init_power_reduction(void);
void decode_statusframe(void);
void xb_join_net();

#define RED_ON()	DEBUG_PORT.OUTSET = DEBUG_RED;
#define RED_OFF()	DEBUG_PORT.OUTCLR = DEBUG_RED;
#define GREEN_ON()	DEBUG_PORT.OUTSET = DEBUG_GREEN;
#define GREEN_OFF()	DEBUG_PORT.OUTCLR = DEBUG_GREEN;

USART_data_t USART_data;

typedef enum XBEE_status_enum
{
	XBEE_SLEEPING 	= 0x01,
	XBEE_JOINED		= 0x02,
	XBEE_ERROR		= 0x03,
} XBee_status_t;

struct XBEE_status_flags
{
	uint8_t	sleeping:1;
	uint8_t joined:1;
	uint8_t error:1;
};

	

int main(void)
{
	// set debug as output
	DEBUG_PORT.DIRSET = 0x03;
	USARTX_PORT.DIRSET = PIN3_bm;

	// initializations
	init_clock();
	xb_init();
	adc_init();
	adc_temp_init();

	// storage variables for sensor readings
	uint16_t amb_light = 0;
	uint16_t chip_temp = 0;
	uint16_t chip_vcc = 0;

	XBee Coord;
	XBee Node1;
	Coord.addr16[0] = 0xff;
	Coord.addr16[1] = 0xfe;
	uint8_t j;
	for(j=0; j<8; j++)
		Coord.addr64[j] = 0;

	xb_set_addr(&Node1, node1_64, 0xfffe);
    
    
    // Enable RXC interrupt on low priority
	USART_RxdInterruptLevel_Set(&USARTX, USART_RXCINTLVL_LO_gc);

	// Enable low level interrupts
	PMIC.CTRL |= PMIC_LOLVLEX_bm;

	sei();

	// TODO : init and wait for device to join a network?
	xb_join_net();

	
	/*
	 *	MAIN LOOP
	 */
	while(1)
	{
		RED_ON();
		
		/*
		 *	Get sensor readings
		 */
		amb_light = 0x0FFF-get_ambient_light();
		chip_temp = get_chip_temperature();
		chip_vcc = get_scaled_vcc();

		/*
		 *	TEMPORARY kludging just to make sure it works out
		 */
		float vcc = ((float)chip_vcc*10)/4096.;
		float temp = ((float)chip_temp*.1294)-273.15;

		// Send a debug string to XBee coordinator
		sprintf(payload, "|AMB:%5d|TEMP:%2.3f|VCC:%2.3f (%d)|", 
									amb_light, (double)temp, (double)vcc, chip_vcc);
		
		payload_length = strlen(payload);

		

		// testing : send to coord AND node1
		xb_wake();
		PORTC.DIRSET = _BV(7);
		while(!xb_is_awake())
			PORTC.OUTSET = _BV(7);

		PORTC.OUTCLR = _BV(7);

		xb_send_array_new(payload, payload_length, &Coord);

		// wait until we've finished sending data before sleeping!
		while((USARTX.STATUS & USART_TXCIF_bm) == 0);

		// TXCIF doesn't seem to work ; wait 1ms instead
		_delay_ms(1);

		xb_sleep();
		//xb_send_array_new(payload, payload_length, &Node1);

		//decode_statusframe();

		RED_OFF();
		_delay_ms(5200);
	}
}

ISR( USARTX_RXC_vect )
{
    DEBUG_PORT.OUTSET = DEBUG_GREEN;
	_delay_us(1);
	DEBUG_PORT.OUTCLR = DEBUG_GREEN;
	

}

// turn on zigbee and wait for it to join a network
void xb_join_net(void)
{
	xb_wake();
	while(1)
	{
		debug_blink(DEBUG_GREEN);
		_delay_ms(100);
	}

}

bool decode_rxframe(void)
{
	// call this function once for every byte received
	// state functions should later be grouped in a structure.

	static uint8_t previous_state = 0;
	static uint16_t frame_size = 0;
	static uint8_t frame_type = 0;
	static bool finished = 0;
	static uint8_t checksum = 0;


	// instance variables for this function
	char byte = 0;

	byte = USART_GetChar(&USARTX);

	switch(previous_state)
	{
		case 0:
			// receive start delimiter
			if(byte == 0x7e)
			{
				previous_state++;
				break;
			} else {
				return false;
			}

		case 1:
			frame_size += ((uint16_t)byte << 8);
			previous_state++;
			break;
		case 2:
			frame_size += byte;
			previous_state++;
			break;
		case 3:
			frame_type = byte;
			previous_state++;
			frame_size--;
		case frame_size:
			checksum = byte;
			previous_state++;
			finished = true;
		default:
			return false;

}


// TODO : Implement RX interrupt (check if packet was transmitted or if not joined)
void decode_statusframe(void)
{
	// check the status of our previous transmissions
	// reminder : 	0x00 -> successfully received
	//				0x22 -> not joined to a network
	//				0x24 -> address not found
	//char byte;
	//uint16_t size = 0;

	// wait for first byte

	// Status frame :
	// 1 byte (0x7e)	Start delimiter
	// 2 bytes 			Message size
	// 1 byte (0x8B)	Frame type (TX Status)
	// 1 byte 			Frame ID
	// 2 byte (0xFFFE)	16-bit destination
	// 1 byte			TX Retry count
	// 1 byte			Delivery status
	// 1 byte			Discovery status
	// 1 byte			Checksum
	
	// empty buffer just to make sure
	while(USART_IsRXComplete(&USARTX))
		USART_GetChar(&USARTX);
}

void init_clock(void)
{
	// TODO : choose a better clock source!!
	// use internal 2MHz RC, no prescaling
	CLKSYS_Prescalers_Config( CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc );
}

// Disable all peripherals that are not used to save power
void init_power_reduction(void)
{
	// General : AES, External Bus Interface, DMA
	PR.PRGEN |= (PR_AES_bm | PR_EBI_bm | PR_DMA_bm);

	// PORT A -> ADC for sensors
	PR.PRPA |= (PR_DAC_bm | PR_AC_bm);

	// PORT B
	PR.PRPB |= (PR_DAC_bm | PR_AC_bm);

	// PORT C -> Sensors port (might need I2C)
	PR.PRPC |= (PR_SPI_bm | PR_HIRES_bm | 
				PR_USART0_bm);

	// PORT D -> USART for ext charger (0) and powerline module (1)
	PR.PRPD |= (PR_TWI_bm | PR_SPI_bm | PR_HIRES_bm | 
				PR_USART1_bm | PR_TC0_bm | PR_TC1_bm);

	// PORT E -> XBee USART (0)
	PR.PRPE |= (PR_TWI_bm | PR_SPI_bm | PR_HIRES_bm | 
				PR_USART1_bm | PR_TC0_bm | PR_TC1_bm);

	// PORT F
	PR.PRPF |= (PR_TWI_bm | PR_SPI_bm | PR_HIRES_bm | 
				PR_USART0_bm | PR_TC0_bm | PR_TC1_bm);

	/***** RTC STUFF *****
	// Set internal 32kHz ULP oscillator as clock source for RTC.
    CLK.RTCCTRL = CLK_RTCSRC_ULP_gc | CLK_RTCEN_bm;

	 // Wait for oscillator to stabilize before setting as RTC clock source.
    do { } while (!( OSC.STATUS & OSC_XOSCRDY_bm ));
    CLK.RTCCTRL = CLK_RTCSRC_TOSC_gc | CLK_RTCEN_bm;

	 // Wait until RTC is ready.
    do { } while ( RTC_Busy() );

    // Configure RTC wakeup period.
    RTC_Initialize( RTC_PERIOD, 0, RTC_PERIOD-1, RTC_PRESCALER_DIV1024_gc );
    
    // Enable RTC compare interrupts.
    RTC_SetCompareIntLevel( RTC_COMPINTLVL_LO_gc );
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei();

	***/


}
