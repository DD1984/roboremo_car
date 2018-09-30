#include <stdbool.h>
#include <Arduino.h>
#include <Ticker.h>

#include "defs.h"
#include "fsm.h"

Ticker bat_ticker;

bool bat_low = false;

void bat_periodic(void)
{
	static int bat_check_cnt = 0;

	if (bat_check_cnt == BAT_CHECK_MAX_CNT) {
		bat_check_cnt = BAT_CHECK_MAX_CNT + 1;
		bat_low = true;

		fsm_signal(BAT_LOW_SIGN);
	}

	if (bat_check_cnt >= BAT_CHECK_MAX_CNT)
		return;

	if (digitalRead(BAT_CHECK_PIN) == HIGH)
		bat_check_cnt++;
	else
		bat_check_cnt = 0;
}

void bat_init(void)
{
	pinMode(BAT_CHECK_PIN, INPUT_PULLUP);
	bat_ticker.attach_ms(BAT_CHECK_INTERVAL, bat_periodic);
}


