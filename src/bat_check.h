#ifndef __BAT_CHECK_H__
#define __BAT_CHECK_H__

bool bat_is_low(void);
void bat_init(void (* action)(void));

#endif
