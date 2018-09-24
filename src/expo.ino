#include <stdint.h>

#include "expo.h"

int16_t calc100to256(int8_t x) // return x*2.56
{
  return ((int16_t)x << 1) + (x >> 1) + (x >> 4);
}

int16_t calc100toRESX(int8_t x) // return x*10.24
{
	int16_t res = ((int16_t)x * 41) >> 2;
	int8_t sign = x < 0 ? 1 : 0;
	x -= sign;
	res -= x >> 6;
	res -= sign;

	return res;
}

int16_t calcRESX1000(int16_t x) // return x/1.024
{
// *1000/1024 = x - x/32 + x/128
	return x - (x >> 5) + (x >> 7);
}

// input parameters;
//  x 0 to 1024;
//  k 0 to 100;
// output between 0 and 1024
unsigned int expou(unsigned int x, unsigned int k)
{
	k = calc100to256(k);

	uint32_t value = (uint32_t) x * x;
	value *= (uint32_t)k;
	value >>= 8;
	value *= (uint32_t)x;

	value >>= 12;
	value += (uint32_t)(256 - k) * x + 128;

	return value >> 8;
}

int expo(int x, int k)
{
	if (k == 0)
		return x;

	int y;
	bool neg = (x < 0);

	if (neg)
		x = -x;

	if (x > (int)RESXu)
		x = RESXu;

	if (k < 0)
		y = RESXu - expou(RESXu - x, -k);
	else
		y = expou(x, k);

	return neg ? -y : y;
}
