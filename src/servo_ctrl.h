#ifndef __SERVO_CTRL_H__
#define __SERVO_CTRL_H__

#include <stdint.h>

void servo_set(int16_t val);
void servo_stop(void);
void servo_init(void);
void servo_trim_plus(void);
void servo_trim_minus(void);

#endif
