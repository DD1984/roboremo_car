#include <stdbool.h>
#include <Arduino.h>
#include <Ticker.h>

#include "led_ctrl.h"
#include "defs.h"

#define REG_SEQ(name) struct led_seq_s name = {name##_arr, ARRAY_SIZE(name##_arr)}

unsigned long led_con_fail_arr[] = {500, 500};
REG_SEQ(led_con_fail);

unsigned long led_con_wifi_arr[] = {167,167,167, 500};
REG_SEQ(led_con_wifi);

unsigned long led_con_wifi_data_arr[] = {1000};
REG_SEQ(led_con_wifi_data);


unsigned long led_bat_low_arr[] = {50, 1450};
REG_SEQ(led_bat_low);


Ticker led_ticker;

struct led_seq_s *led_cur_seq = NULL;
struct led_seq_s *led_new_seq = NULL;

void led_ctrl(bool first_step)
{
	if (led_cur_seq == NULL) {
		digitalWrite(LED_PIN, HIGH);
		return;
	}

	static bool led_state = false;
	static int cur_step = 0;
	static unsigned long led_next_time = 0;

	unsigned long now_time = millis();

	if (first_step) {
		led_next_time = now_time;
		cur_step = led_cur_seq->size;
	}

	if (now_time >= led_next_time) {
		if (++cur_step >= led_cur_seq->size) {
			cur_step = 0;
			led_state = false;
		}
		led_next_time = now_time + led_cur_seq->arr[cur_step];

		led_state = !led_state;
		if (led_state)
			digitalWrite(LED_PIN, LOW);
		else
			digitalWrite(LED_PIN, HIGH);
	}
}

void led_set_seq(struct led_seq_s *s)
{
	led_cur_seq = s;
	led_ctrl(true);
}

void led_init(void)
{
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	led_ticker.attach_ms(10, led_ctrl, false);
}
