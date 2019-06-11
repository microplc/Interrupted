#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "atomic.h"
#include "device.h"
#include "serial.h"
#include "serialout.h"

static SerialOut_ *device;

bool SerialOut_::begin() {
	device = this;
	init();
	UCSR0B |= _BV(TXEN0);
	return false;
}

void SerialOut_::_enable(bool enable) {
	if (enable)
		UCSR0B |= _BV(TXCIE0);
	else
		UCSR0B &= ~_BV(TXCIE0);
}

ISR(USART_TX_vect)
{
	if (device) {
		uint8_t b;
		if (device->next(b))
			UDR0 = b;
	}
}
