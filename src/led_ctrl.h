#ifndef __LED_CTRL_H__
#define __LED_CTRL_H__

#include <stdbool.h>

struct led_seq_s {
	unsigned long *arr;
	int size;
};

#define EXPORT_SEQ(name) extern led_seq_s name

EXPORT_SEQ(led_con_fail);
EXPORT_SEQ(led_con_wifi);
EXPORT_SEQ(led_con_wifi_data);
EXPORT_SEQ(led_bat_low);

#define LED_CON_FAIL &led_con_fail
#define LED_CON_WIFI &led_con_wifi
#define LED_CON_WIFI_DATA &led_con_wifi_data
#define LED_BAT_LOW &led_bat_low

void led_ctrl(bool first_step);
#define led_periodic() led_ctrl(false)
void led_set_seq(struct led_seq_s *s);
void led_init(void);

#endif
