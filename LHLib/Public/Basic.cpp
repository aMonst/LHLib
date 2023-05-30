#include "Basic.h"
#include <float.h>
#include <math.h>

bool IsFloatBigger(float f1, float f2)
{
	return (f1 - f2) > FLT_EPSILON;
}

bool IsFloatSmaller(float f1, float f2)
{
	return (f1 - f2) < -FLT_EPSILON;
}

bool IsFloatZero(float f1)
{
	return IsFloatEqual(f1, 0.0);
}

bool IsFloatEqual(float f1, float f2)
{
	return fabs(f1 - f2) < FLT_EPSILON;
}