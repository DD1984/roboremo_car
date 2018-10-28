#ifndef __SERVO_CTRL_H__
#define __SERVO_CTRL_H__

#include <stdint.h>

void servo_set(uint8_t num, int16_t val);
void servo_stop(uint8_t num);
void servo_stop_all(void);
void servo_init(void);
void servo_trim_action(int16_t val);

#endif
