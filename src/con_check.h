#ifndef __CON_CHECK_H__
#define __CON_CHECK_H__

#define CON_FAIL 0
#define CON_WIFI 1
#define CON_WIFI_DATA 2

void init_con_check(void (* change_action)(void));
int get_con_state(void);
void network_data_available(void);

#endif
