#include <stdint.h>
#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>

#include "defs.h"
#include "expo.h"

Servo servo;

#define TRIM_ADDR 0
#define TRIM_DELTA 5

typedef struct {
	int16_t val;
	uint32_t crc;
} trim_t;

int16_t servo_trim = 0;

void servo_set(int16_t val)
{
	val = ((calcRESX1000(expo(calc100toRESX(val), SERVO_EXPO)) + 1000) >> 1) + 1000;
	servo.writeMicroseconds(val + servo_trim);
}

void servo_stop(void)
{
	servo.detach();
	pinMode(SERVO_PIN, OUTPUT);
	digitalWrite(SERVO_PIN, LOW);
}

uint32_t calculateCRC32(const uint8_t *data, size_t length)
{
	uint32_t crc = 0xffffffff;
	while (length--) {
		uint8_t c = *data++;
		for (uint32_t i = 0x80; i > 0; i >>= 1) {
			bool bit = crc & 0x80000000;
			if (c & i) {
				bit = !bit;
			}
			crc <<= 1;
			if (bit) {
				crc ^= 0x04c11db7;
			}
		}
	}
	return crc;
}

int16_t servo_trim_load(void)
{
	trim_t trim;
	EEPROM.get(TRIM_ADDR, trim);
	uint32_t crc = calculateCRC32((uint8_t *)&trim.val, sizeof(trim.val));

	if (crc == trim.crc) {
		Serial.printf("servo trim: %d\n", trim.val);
		return trim.val;
	}

	Serial.printf("bad servo trim crc - calc:0x08%x stored::0x08%x\n", crc, trim.crc);

	return 0;
}

void servo_trim_save(int16_t val)
{
	trim_t trim;
	trim.val = val;
	trim.crc = calculateCRC32((uint8_t *)&trim.val, sizeof(trim.val));

	Serial.printf("save trim: val: %d crc: 0x08%x\n", trim.val, trim.crc);

	EEPROM.put(TRIM_ADDR, trim);
	EEPROM.commit();
}

void servo_init(void)
{
	servo.attach(SERVO_PIN, 1000, 2000);
	EEPROM.begin(sizeof(trim_t));
	servo_trim = servo_trim_load();
}

void servo_trim_action(int16_t val)
{
	if (val > 0)
		servo_trim += TRIM_DELTA;
	if (val < 0)
		servo_trim -= TRIM_DELTA;

	if (abs(servo_trim) < TRIM_DELTA)
		servo_trim = 0;

	if (val != 0)
		servo_trim_save(servo_trim);
}
