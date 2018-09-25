#include <stdbool.h>
#include <Arduino.h>
#include <Ticker.h>

#include "defs.h"

Ticker bat_ticker;

bool bat_low = false;

void bat_periodic(void (* action)(void))
{
	static int bat_check_cnt = 0;

	if (bat_check_cnt == BAT_CHECK_MAX_CNT) {
		bat_check_cnt = BAT_CHECK_MAX_CNT + 1;
		action();
		bat_low = true;
	}

	if (bat_check_cnt >= BAT_CHECK_MAX_CNT)
		return;

	if (digitalRead(BAT_CHECK_PIN) == HIGH)
		bat_check_cnt++;
	else
		bat_check_cnt = 0;
}

bool bat_is_low(void)
{
	return bat_low;
}

void bat_init(void (* action)(void))
{
	pinMode(BAT_CHECK_PIN, INPUT_PULLUP);
	bat_ticker.attach_ms(BAT_CHECK_INTERVAL, bat_periodic, action);
}


