#include <stdint.h>
#include <Arduino.h>
#include <Servo.h>

#include "defs.h"
#include "expo.h"

Servo servo;

void servo_set(int16_t val)
{
	val = ((calcRESX1000(expo(calc100toRESX(val), SERVO_EXPO)) + 1000) >> 1) + 1000;
	servo.writeMicroseconds(val);
}

void servo_stop(void)
{
	servo.detach();
	pinMode(SERVO_PIN, OUTPUT);
	digitalWrite(SERVO_PIN, LOW);
}

void servo_init(void)
{
	servo.attach(SERVO_PIN, 1000, 2000);
}
