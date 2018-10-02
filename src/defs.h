#ifndef __DEFS_H__
#define __DEFS_H__

#define WIFI_SSID "mywifi"
#define WIFI_PSK ""
#define WIFI_CHANNEL 1

#define PORT 9876

#define CON_CHECK_INTERVAL 500 //ms

#define SERVO_EXPO 50
#define SPEED_EXPO 55

// H-bridge gpios
#define H_BCK_PIN 13
#define H_FWD_PIN 14
#define H_PWM_PIN 5

#define BAT_CHECK_PIN 12
#define BAT_CHECK_INTERVAL 100 //ms
#define BAT_CHECK_MAX_CNT  10

#define SERVO_PIN 4

#define LED_PIN 2

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#endif
