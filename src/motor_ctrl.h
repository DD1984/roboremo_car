#ifndef __MOTOR_CTRL_H__
#define __MOTOR_CTRL_H__

#include <stdint.h>

void motor_set(int16_t val);
void motor_brake(void);
void motor_init(void);

#endif
