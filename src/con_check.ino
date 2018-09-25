#include <Ticker.h>
#include <stdbool.h>

#include "defs.h"
#include "con_check.h"

#define NONE 0
#define STA_CON 1
#define STA_DISCON 2
#define DATA_AVAIL 3
#define DATA_LOSS 4


int FSM_table[3][5] = {
	[CON_FAIL] = {[NONE] = CON_FAIL, [STA_CON] = CON_WIFI, [STA_DISCON] = CON_FAIL, [DATA_AVAIL] = CON_WIFI_DATA, [DATA_LOSS] = CON_FAIL},
	[CON_WIFI] = {[NONE] = CON_WIFI, [STA_CON] = CON_WIFI, [STA_DISCON] = CON_FAIL, [DATA_AVAIL] = CON_WIFI_DATA, [DATA_LOSS] = CON_WIFI},
	[CON_WIFI_DATA] = {[NONE] = CON_WIFI_DATA, [STA_CON] = CON_WIFI_DATA, [STA_DISCON] = CON_FAIL, [DATA_AVAIL] = CON_WIFI_DATA, [DATA_LOSS] = CON_WIFI},
};

Ticker con_ticker;

int current_state = CON_FAIL;
bool data_state = false;

int get_con_state(void)
{
	return current_state;
}

void network_data_available(void)
{
	data_state = true;
}


void con_periodic(void (* change_action)(void))
{
	int signal = NONE;

	static int last_sta_num = 0;
	static bool last_data_state = false;

	int cur_sta_num = WiFi.softAPgetStationNum();

	if (cur_sta_num > 0 && last_sta_num == 0)
		signal = STA_CON;
	if (cur_sta_num == 0 && last_sta_num > 0)
		signal = STA_DISCON;

	if (signal == NONE) {
		if (data_state == true && last_data_state == false)
			signal = DATA_AVAIL;
		if (data_state == false && last_data_state == true)
			signal = DATA_LOSS;
	}

	last_sta_num = cur_sta_num;
	last_data_state = data_state;

	data_state = false;


	int new_state = FSM_table[current_state][signal];

	if (new_state != current_state) {
		current_state = new_state;
		change_action();
	}
}

void init_con_check(void (* change_action)(void))
{
	con_ticker.attach_ms(CON_CHECK_INTERVAL, con_periodic, change_action);
}

