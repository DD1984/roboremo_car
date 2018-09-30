#include <Ticker.h>
#include <stdbool.h>

#include "defs.h"
#include "fsm.h"
#include "con_check.h"

Ticker con_ticker;

bool data_state = false;


void con_check_data_available(void)
{
	data_state = true;
}

void con_periodic(void)
{
	static int last_sta_num = 0;
	static bool last_data_state = false;

	int cur_sta_num = WiFi.softAPgetStationNum();

	if (cur_sta_num > 0 && last_sta_num == 0)
		fsm_signal(STA_CON_SIGN);
	if (cur_sta_num == 0 && last_sta_num > 0)
		fsm_signal(STA_DISCON_SIGN);

	if (data_state == true && last_data_state == false)
		fsm_signal(DATA_AVAIL_SIGN);
	if (data_state == false && last_data_state == true)
		fsm_signal(DATA_LOSS_SIGN);

	last_sta_num = cur_sta_num;
	last_data_state = data_state;

	data_state = false;
}

void con_check_init(void)
{
	con_ticker.attach_ms(CON_CHECK_INTERVAL, con_periodic);
}

