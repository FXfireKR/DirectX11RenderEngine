#pragma once
#include <math.h>

static float saturate(float f)
{
	if (f < 0.f) return 0.f;
	if (f > 1.f) return 1.f;
	return f;
}

static float smooth(float f)
{
	f = saturate(f);
	return f * f * (3.f - 2.f * f);
}

static float pulse(float x, float c, float h)
{
	if (h <= 0.f)
		return 0.f;

	float t = 1.f - (std::fabsf(x - c) / h);
	return smooth(t);
}