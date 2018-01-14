#include "imath.h"
#include <math.h>
float SIN_TABLE[2048];
void initMath()
{
	for (int i = 0; i < 2048; i++)
	{
		SIN_TABLE[i] = (float) sin(i / 1024.0f * M_PI);
	}
}
float isin(int x)
{
	return SIN_TABLE[x & 2047];
}
float icos(int x)
{
	return isin(x + 512);
}
