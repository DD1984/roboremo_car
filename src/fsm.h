#ifndef __FSM_H__
#define __FSM_H__

// fsm states
#define CON_FAIL_STATE      0
#define CON_WIFI_STATE      1
#define CON_WIFI_DATA_STATE 2
#define BAT_LOW_STATE       3

//fsm signals
#define STA_CON_SIGN    0
#define STA_DISCON_SIGN 1
#define DATA_AVAIL_SIGN 2
#define DATA_LOSS_SIGN  3
#define BAT_LOW_SIGN    4

void fsm_signal(int signal);
int fsm_get_state(void);
void fsm_init(void (* change_action)(int new_state));

#endif
