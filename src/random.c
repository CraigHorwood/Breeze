#include "random.h"
#include <stdlib.h>
#include <time.h>
void initRandom()
{
	srand(time(NULL));
}
int next()
{
	return rand();
}
int nextInt(int max)
{
	return next() % max;
}
float nextFloat()
{
	return (float) next() / RAND_MAX;
}
