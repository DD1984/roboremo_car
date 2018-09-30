#include "fsm.h"

int fsm_table[4][5] = {
    [CON_FAIL_STATE] = {
        [STA_CON_SIGN] =    CON_WIFI_STATE,
        [STA_DISCON_SIGN] = CON_FAIL_STATE,
        [DATA_AVAIL_SIGN] = CON_WIFI_DATA_STATE,
        [DATA_LOSS_SIGN] =  CON_FAIL_STATE,
        [BAT_LOW_SIGN] =    BAT_LOW_STATE,
    },
    [CON_WIFI_STATE] = {
        [STA_CON_SIGN] =    CON_WIFI_STATE,
        [STA_DISCON_SIGN] = CON_FAIL_STATE,
        [DATA_AVAIL_SIGN] = CON_WIFI_DATA_STATE,
        [DATA_LOSS_SIGN] =  CON_WIFI_STATE,
        [BAT_LOW_SIGN] =    BAT_LOW_STATE,
    },
    [CON_WIFI_DATA_STATE] = {
        [STA_CON_SIGN] =    CON_WIFI_DATA_STATE,
        [STA_DISCON_SIGN] = CON_FAIL_STATE,
        [DATA_AVAIL_SIGN] = CON_WIFI_DATA_STATE,
        [DATA_LOSS_SIGN] =  CON_WIFI_STATE,
        [BAT_LOW_SIGN] =    BAT_LOW_STATE,
    },
    [BAT_LOW_STATE] = {
        [STA_CON_SIGN] =    BAT_LOW_STATE,
        [STA_DISCON_SIGN] = BAT_LOW_STATE,
        [DATA_AVAIL_SIGN] = BAT_LOW_STATE,
        [DATA_LOSS_SIGN] =  BAT_LOW_STATE,
        [BAT_LOW_SIGN] =    BAT_LOW_STATE,
    },
};

int fsm_state = CON_FAIL_STATE;
void (* fsm_change_action_func)(int new_state) = NULL;

void fsm_signal(int signal)
{
	int new_state = fsm_table[fsm_state][signal];

	if (new_state != fsm_state) {
		fsm_state = new_state;
		if (fsm_change_action_func)
			fsm_change_action_func(new_state);
	}
}

int fsm_get_state(void)
{
	return fsm_state;
}

void fsm_init(void (* change_action)(int new_state))
{
	fsm_change_action_func = change_action;
}
