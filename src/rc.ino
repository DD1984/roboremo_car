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
#include "expo.h"
#include "servo_ctrl.h"
#include "motor_ctrl.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

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


void setup()
{
	delay(1000);

	Serial.begin(115200);

	IPAddress ip(192, 168, 0, 1);
	IPAddress netmask(255, 255, 255, 0);

	WiFi.softAPConfig(ip, ip, netmask);
	WiFi.softAP(WIFI_SSID, WIFI_PSK, WIFI_CHANNEL, 0, 1); // 1 client possible

	udp.begin(PORT); // start UDP server

	Serial.println("ESP8266 RC receiver (UDP) 1.0 powered by RoboRemo");
	Serial.println((String)"SSID: " + WIFI_SSID + "  PASS: " + WIFI_PSK);
	Serial.println((String)"RoboRemo app must connect to " + ip.toString() + ":" + PORT);

	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	pinMode(BAT_CHECK_PIN, INPUT_PULLUP);
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

enum {
	CON_FAIL,
	CON_WIFI,
	CON_WIFI_DATA,
};

int con_state = CON_FAIL;

void start_ctrl(void)
{
	Serial.printf("start ctrl\n");

	servo_init();
	motor_init();
}

void stop_ctrl(void)
{
	Serial.print("stop ctrl\n");

	servo_stop();
	motor_brake();
}


void loop()
{
	static int bat_check_cnt = 0;
	static int bat_last_time = 0;
	int bat_now_time = millis();
	if (bat_now_time - bat_last_time > BAT_CHECK_INTERVAL) {
		bat_last_time = bat_now_time;

		if (bat_check_cnt == BAT_CHECK_MAX_CNT) {
			servo_stop();
			motor_brake();
			//WiFi.softAPdisconnect(true);
			WiFi.mode(WIFI_OFF);
			WiFi.forceSleepBegin();

			bat_check_cnt++;
		}

		if (bat_check_cnt >= BAT_CHECK_MAX_CNT) {
			static int bat_led_cnt = 0;
			
			if (bat_led_cnt == 0)
				digitalWrite(LED_PIN, LOW);
			else
				digitalWrite(LED_PIN, HIGH);
			
			if (++bat_led_cnt >= 20)
				bat_led_cnt = 0;

			return;
		}

		if (digitalRead(BAT_CHECK_PIN) == HIGH)
			bat_check_cnt++;
		else
			bat_check_cnt = 0;
	}
	if (bat_check_cnt >= BAT_CHECK_MAX_CNT)
		return;


	static int con_last_time = 0;
	static bool data_available = false;

	int con_now_time = millis();
	if (con_now_time - con_last_time > CON_CHECK_INTERVAL) {

		//Serial.print("check connection\n");

		con_last_time = con_now_time;

		static int last_sta_num = 0;
		int cur_sta_num = WiFi.softAPgetStationNum();

		if (cur_sta_num > 0 && last_sta_num == 0) {
			Serial.print("client connected\n");
			con_state = CON_WIFI;
		}
		if (cur_sta_num == 0 && last_sta_num > 0) {
			Serial.print("client disconnected\n");
			if (con_state == CON_WIFI_DATA)
				stop_ctrl();
			con_state = CON_FAIL;
		}

		last_sta_num = cur_sta_num;

		if (con_state != CON_FAIL) {
			static bool last_data_available = false;

			if (con_state == CON_WIFI && !last_data_available && data_available) {
				Serial.print("client data available\n");
				con_state = CON_WIFI_DATA;
				start_ctrl();
			}

			if (con_state == CON_WIFI_DATA && last_data_available && !data_available) {
				Serial.print("client data timeot expired\n");
				con_state = CON_WIFI;
				stop_ctrl();
			}

			last_data_available = data_available;
			data_available = false;
		}
		
		static int con_led_cnt = 0;
		if (con_state != CON_WIFI_DATA) {
			if (con_led_cnt == 0)
				digitalWrite(LED_PIN, HIGH);
			else
				digitalWrite(LED_PIN, LOW);
			con_led_cnt = !con_led_cnt;
		}
		else 
			digitalWrite(LED_PIN, LOW);
	}

	if (con_state == CON_FAIL)
		return;

	int pkt_size = udp.parsePacket();
	if (pkt_size) {
		//Serial.printf("Received %d bytes from %s, port %d\n", pkt_size, udp.remoteIP().toString().c_str(), udp.remotePort());
		char pkt_buf[64];
		int len = udp.read(pkt_buf, sizeof(pkt_buf));
		if (len > 0) {
			//Serial.printf("UDP packet contents: %s\n", pkt_buf);

			data_available = true;

			if (con_state == CON_WIFI)
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

			int16_t servo_val;

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
