#ifndef __EXPO_H__
#define __EXPO_H__

#define RESXu 1024u

int16_t calc100to256(int8_t x);
int16_t calc100toRESX(int8_t x);
int16_t calcRESX1000(int16_t x);
unsigned int expou(unsigned int x, unsigned int k);
int expo(int x, int k);

#endif
