#include <avr/power.h>
#include <avr/sleep.h>

#include <Arduino.h>
#include <stdarg.h>

#include "device.h"

void Devices::begin() {
	// turn off ADC and analog comparator
	ADCSRA &= ~bit(ADEN);
	ACSR &= ~bit(ACD);
	power_adc_disable();	// FIXME: power_all_disable()?

	for (int i = 0; i <= 10; i++) {
		pinMode(i, INPUT);
		digitalWrite(i, LOW);
	}

	for (int i = 0; i < _n; i++) {
		_devices[i]->begin();
		_devices[i]->enable();
	}
	sei();
}

unsigned Device::sleepmode() {
	return SLEEP_MODE_PWR_DOWN;
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

	// this exists on later avrlibs but not the one shipped with
	// arduino. also the later arduino core won't compile under
	// the latest avr-gcc so we're stuck with arduino10!
// FIXME
//	sleep_bod_disable();
	sei();
	sleep_cpu();
	sleep_disable();

	goto again;
}