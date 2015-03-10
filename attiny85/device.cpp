#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <Arduino.h>

#include "device.h"

unsigned Device::sleepmode() {
	return SLEEP_MODE_PWR_DOWN;
}

void Devices::begin() {
	// "...[it] is therefore required to turn off the watchdog
	// early during program startup..." (from avr/wdt.h)
	wdt_disable();
	
	// turn off ADC and analog comparator
	ADCSRA = 0;
	ACSR |= bit(ACD);
	power_adc_disable();    // FIXME: power_all_disable()?
	
	// turn off the brown-out detector
	MCUCR |= _BV(BODS) | _BV(BODSE);
	
	for (int i = 0; i <= 10; i++)
	        digitalWrite(i, LOW);
	
	for (int i = 0; i < _n; i++)
	        _devices[i]->enable(_devices[i]->begin());
	
	sei();
}

// required because there's no defined ordering of modes...
static unsigned update_mode(unsigned m, unsigned mode) {
	switch (mode) {
	case SLEEP_MODE_IDLE:
		return mode;

	case SLEEP_MODE_ADC:
		if (m != SLEEP_MODE_IDLE)
			return mode;
		break;
	case SLEEP_MODE_PWR_SAVE:
		if (m != SLEEP_MODE_IDLE && m != SLEEP_MODE_ADC)
			return mode;
		break;
	case SLEEP_MODE_PWR_DOWN:
		break;
	}
	return m;
}

int Devices::select() {
again:
	// so we don't miss an interrupt while checking...
	cli();
	unsigned mode = SLEEP_MODE_PWR_DOWN;
	for (int i = 0; i < _n; i++) {
		Device *d = _devices[i];
		if (d->is_ready()) {
			sei();
			return d->id();
		}
		if (d->is_enabled())
			mode = update_mode(mode, d->sleepmode());
	}

	set_sleep_mode(mode);
	sleep_enable();

	// arduino 1.5.8 finally updated the avr toolchain
	sleep_bod_disable();
	sei();
	sleep_cpu();
	sleep_disable();

	goto again;
}