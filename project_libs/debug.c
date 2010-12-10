#include <avr/io.h>
#include "hardware.h"
#include "debug.h"

#include <util/delay.h>

void debug_blink(uint8_t color)
{
	DEBUG_PORT.OUTSET = color;
	_delay_ms(50);
	DEBUG_PORT.OUTCLR = color;
}
