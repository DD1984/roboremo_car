/*
 * RC receiver for controlling RC car:
 * - servo
 * - motor with brake - H-bridge, controlled by esp8266
 * using an ESP8266 and an Android phone with RoboRemo app
 * 
 * are used:
 * - UDP connection for minimal delay
 * - exponential regulation for best precision of control
 * - battery voltage check for save li-po
 * - connection check for safe use
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "defs.h"
#include "con_check.h"
#include "expo.h"
#include "servo_ctrl.h"
#include "motor_ctrl.h"
#include "led_ctrl.h"
#include "bat_check.h"

enum {
	UNKNOWN = -1,

	SERVO,
	SPEED,
	BRAKE,
};

typedef struct {
	int16_t min;
	int16_t max;
	int16_t val;
} ctrl_param_t;

typedef struct {
	char *roboremo_id;
	ctrl_param_t *param;
} ctrl_t;


ctrl_param_t servo_param = {-100, 100, 0};
ctrl_param_t speed_param = {-100, 100, 0};

ctrl_t controls[] = {
	[SERVO] =		{"servo", &servo_param},
	[SPEED] =		{"speed", &speed_param},
	[BRAKE] =		{"brake", NULL},
};

#define CTRL_VAL(x) (controls[x].param->val)

#define servo_val		(CTRL_VAL(SERVO))
#define speed_val		(CTRL_VAL(SPEED))

WiFiUDP udp;

void start_ctrl(void)
{
	servo_init();
	motor_init();
}

void stop_ctrl(void)
{
	servo_stop();
	motor_brake();
}

void bat_low_action(void)
{
	stop_ctrl();

	//WiFi.softAPdisconnect(true);
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();

	led_set_seq(LED_BAT_LOW);
}

void con_change_action(void)
{
	int state = get_con_state();
	switch (state) {
		case CON_FAIL:
			led_set_seq(LED_CON_FAIL);
			stop_ctrl();
		break;
		case CON_WIFI:
			led_set_seq(LED_CON_WIFI);
			stop_ctrl();
		break;
		case CON_WIFI_DATA:
			led_set_seq(LED_CON_WIFI_DATA);
			start_ctrl();

		break;
	}
}

void setup()
{
	Serial.begin(115200);

	IPAddress ip(192, 168, 0, 1);
	IPAddress netmask(255, 255, 255, 0);

	WiFi.softAPConfig(ip, ip, netmask);
	WiFi.softAP(WIFI_SSID, WIFI_PSK, WIFI_CHANNEL, 0, 1); // 1 client possible

	udp.begin(PORT); // start UDP server

	Serial.println("ESP8266 RC receiver (UDP) 1.0 powered by RoboRemo");
	Serial.println((String)"SSID: " + WIFI_SSID + "  PASS: " + WIFI_PSK);
	Serial.println((String)"RoboRemo app must connect to " + ip.toString() + ":" + PORT);

	led_init();
	led_set_seq(LED_CON_FAIL);

	bat_init(bat_low_action);

	init_con_check(con_change_action);
}

ctrl_t *get_ctrl(int id)
{
	if (id >= 0 && id < ARRAY_SIZE(controls))
		return &controls[id];

	return NULL;
}

//return ctrl id
int parse_pkt(char *pkt_buf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(controls); i++) {
		if (!strncmp(pkt_buf, controls[i].roboremo_id, strlen(controls[i].roboremo_id))
				&& (!controls[i].param || pkt_buf[strlen(controls[i].roboremo_id)] == ' ')) {

			if (controls[i].param) {

				int val = atoi(pkt_buf + strlen(controls[i].roboremo_id));
				if (val < controls[i].param->min || val > controls[i].param->max)
					return UNKNOWN;

				if (controls[i].param->val == val)
					return UNKNOWN;

				controls[i].param->val = val;
			}

			return i;
		}
	}

	return UNKNOWN;
}

void loop()
{
	if (bat_is_low())
		return;

	if (get_con_state() == CON_FAIL)
		return;

	int pkt_size = udp.parsePacket();
	if (pkt_size) {
		//Serial.printf("Received %d bytes from %s, port %d\n", pkt_size, udp.remoteIP().toString().c_str(), udp.remotePort());
		char pkt_buf[64];
		int len = udp.read(pkt_buf, sizeof(pkt_buf));
		if (len > 0) {
			//Serial.printf("UDP packet contents: %s\n", pkt_buf);

			network_data_available();

			if (get_con_state() == CON_WIFI)
				return;

			pkt_buf[len] = 0;

			int id = parse_pkt(pkt_buf);
			ctrl_t *ctrl = get_ctrl(id);

			if (ctrl) {
				Serial.printf("ctrl: \"%s\"", ctrl->roboremo_id);
				if (ctrl->param)
					Serial.printf("val: %d", ctrl->param->val);
				Serial.printf("\n");
			}

			switch (id) {
				case SERVO:
					servo_set(servo_val);
				break;
				case SPEED:
					motor_set(speed_val);
				break;
				case BRAKE:
					if (speed_val == 0)
						motor_brake();
				break;
			}
		}
	}
}
