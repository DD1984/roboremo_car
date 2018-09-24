#include <stdint.h>
#include <Arduino.h>

#include "defs.h"
#include "expo.h"

void motor_set(int16_t val)
{
	val = calc100toRESX(val);

	if (val < 0) {
		val = -val;
		digitalWrite(H_FWD_PIN, LOW);
		digitalWrite(H_BCK_PIN, HIGH);
	}
	else if (val > 0) {
		digitalWrite(H_FWD_PIN, HIGH);
		digitalWrite(H_BCK_PIN, LOW);
	}
	else {
		digitalWrite(H_FWD_PIN, LOW);
		digitalWrite(H_BCK_PIN, LOW);
	}

	val = expo(val, SPEED_EXPO);

	if (val > 1023)
		val = 1023;

	analogWrite(H_PWM_PIN, 1023 - val);
}

void motor_brake(void)
{
	digitalWrite(H_FWD_PIN, LOW);
	digitalWrite(H_BCK_PIN, LOW);
	digitalWrite(H_PWM_PIN, LOW);
}

void motor_init(void)
{
	pinMode(H_FWD_PIN, OUTPUT);
	digitalWrite(H_FWD_PIN, LOW);

	pinMode(H_BCK_PIN, OUTPUT);
	digitalWrite(H_BCK_PIN, LOW);

	pinMode(H_PWM_PIN, OUTPUT);
	digitalWrite(H_PWM_PIN, LOW);
}
