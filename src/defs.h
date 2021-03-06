#ifndef __DEFS_H__
#define __DEFS_H__

#define WIFI_SSID "mywifi"
#define WIFI_PSK ""
#define WIFI_CHANNEL 1

#define PORT 9876

#define CON_CHECK_INTERVAL 500 //ms

#define SPEED_EXPO 55

// H-bridge gpios
#define H_BCK_PIN 14
#define H_FWD_PIN 12
#define H_PWM_PIN 2

#define AUTO_BRAKE //brake motor when motor channel == 0

#define BAT_CHECK_PIN 13
#define BAT_CHECK_INTERVAL 100 //ms
#define BAT_CHECK_MAX_CNT  10

#define SERVO_PINS {4, 5}
#define SERVO_EXPO {55, 0}

#define LED_PIN 0

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#endif
