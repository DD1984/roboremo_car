#include <stdint.h>
#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>

#include "defs.h"
#include "expo.h"

#define TRIM_ADDR 0
#define TRIM_DELTA 5

int servo_pins[] = SERVO_PINS;
int servo_expo[ARRAY_SIZE(servo_pins)] = SERVO_EXPO;
int16_t servo_trim[ARRAY_SIZE(servo_pins)];
Servo servo[ARRAY_SIZE(servo_pins)];

typedef struct {
	int16_t vals[ARRAY_SIZE(servo_pins)];
	uint32_t crc;
} trim_store_t;

void servo_set(uint8_t num, int16_t val)
{
	val = ((calcRESX1000(expo(calc100toRESX(val), servo_expo[num])) + 1000) >> 1) + 1000;
	servo[num].writeMicroseconds(val + servo_trim[num]);
}

void servo_stop(uint8_t num)
{
	servo[num].detach();
	pinMode(servo_pins[num], OUTPUT);
	digitalWrite(servo_pins[num], LOW);
}

void servo_stop_all(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(servo_pins); i++)
		servo_stop(i);
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

void servo_trim_load(void)
{
	trim_store_t trim;
	EEPROM.get(TRIM_ADDR, trim);
	uint32_t crc = calculateCRC32((uint8_t *)&trim.vals[0], sizeof(trim.vals));

	if (crc == trim.crc) {
		int i;
		Serial.printf("servo trim: ");
		for (i = 0; i < ARRAY_SIZE(servo_pins); i++)
			Serial.printf("%d ", trim.vals[i]);
		Serial.printf("\n");

		memcpy(servo_trim, &trim.vals[0], sizeof(servo_trim));

		return;
	}

	bzero(servo_trim, sizeof(servo_trim));

	Serial.printf("bad servo trim crc - calc:0x08%x stored::0x08%x\n", crc, trim.crc);
}

void servo_trim_save(void)
{
	trim_store_t trim;
	memcpy(&trim.vals[0], servo_trim, sizeof(servo_trim));

	trim.crc = calculateCRC32((uint8_t *)&trim.vals[0], sizeof(trim.vals));

	int i;
	Serial.printf("save trim: vals: ");
	for (i = 0; i < ARRAY_SIZE(servo_pins); i++)
		Serial.printf("%d ", trim.vals[i]);

	Serial.printf("crc: 0x08%x\n", trim.crc);

	EEPROM.put(TRIM_ADDR, trim);
	EEPROM.commit();
}

void servo_init(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(servo_pins); i++)
		servo[i].attach(servo_pins[i], 1000, 2000);
	
	EEPROM.begin(sizeof(trim_store_t));
	servo_trim_load();
}

void servo_trim_action(int16_t val)
{
	if (val > 0)
		servo_trim[0] += TRIM_DELTA;
	if (val < 0)
		servo_trim[0] -= TRIM_DELTA;

	if (abs(servo_trim[0]) < TRIM_DELTA)
		servo_trim[0] = 0;

	if (val != 0)
		servo_trim_save();
}
