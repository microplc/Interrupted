#include <Interrupted.h>

Port p3, p4;
Pin led5(LED5, p3);
Pin led6(LED6, p3);
Pin led7(LED7, p3);
Pin led8(LED8, p3);

Pin *leds[] = { &led5, &led6, &led7, &led8 };
Pin push2(PUSH2, p4);
Devices devices;

void setup()
{
	pinMode(RED_LED, OUTPUT);
	pinMode(GREEN_LED, OUTPUT);

	pinMode(LED5, OUTPUT);
	pinMode(LED6, OUTPUT);
	pinMode(LED7, OUTPUT);
	pinMode(LED8, OUTPUT);
	pinMode(PUSH2, INPUT_PULLUP);

	for (int i = 0; i < 4; i++) {
		digitalWrite(LED5 + i, LOW);
		devices.add(*leds[i]);
	}

	devices.add(push2);
	devices.begin();
}

void loop()
{
	static int i;

	switch (devices.select()) {
	case LED5:
	case LED7:
		digitalWrite(RED_LED, HIGH);
		digitalWrite(GREEN_LED, LOW);
		break;
	case LED6:
	case LED8:
		digitalWrite(RED_LED, LOW);
		digitalWrite(GREEN_LED, HIGH);
		break;
	case PUSH2:
		if (!push2.is_on()) {
			digitalWrite(LED5+i, !digitalRead(LED5+i));	
			i++;
			if (i == 4)
				i = 0;
		}
		break;
	}
}
