#ifndef UNITY_BUILD
#include "math.h"
#include "vector.h"
#endif

float vector::LengthSquared()
{
	return sqrt(x*x + y*y + z*z);
}

float vector::Length()
{
	return sqrt(x*x + y*y + z*z);
}

float vector::DotProduct(vector &v1, vector v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}