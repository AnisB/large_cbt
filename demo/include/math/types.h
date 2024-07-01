#pragma once

// External includes
#include <math.h>
#include <stdint.h>

// Constants
#define QUARTER_PI 0.78539816339
#define HALF_PI 1.57079632679
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define INV_PI 0.31830988618
#define DEG_TO_RAD (PI / 180.0)

struct half2
{
	uint16_t x, y;
};

struct half3
{
	uint16_t x, y, z;
};

struct half4
{
	uint16_t x, y, z, w;
};

struct float2
{
	float x,y;
};

struct float3
{
	float x, y, z;
};

struct float4
{
	float x, y, z, w;
};

struct double2
{
	double x, y;
};

struct double3
{
	double x, y, z;
};

struct double4
{
	double x, y, z, w;
};

struct uint2
{
	uint32_t x,y;
};

struct uint3
{
	uint32_t x, y, z;
};

struct uint4
{
	uint32_t x,y,z,w;
};

struct int2
{
	int32_t x, y;
};

struct int3
{
	int32_t x, y, z;
};

struct int4
{
	int32_t x, y, z, w;
};

struct float2x2
{
	union
	{
		float m[4];
		float rc[2][2];
	};
};

struct float3x3
{
	union
	{
		float m[9];
		float rc[3][3];
	};
};

struct float4x4
{
	union
	{
		float m[16];
		float rc[4][4];
	};
};

struct double2x2
{
	union
	{
		double m[4];
		double rc[2][2];
	};
};

struct double3x3
{
	union
	{
		double m[9];
		double rc[3][3];
	};
};

struct double4x4
{
	union
	{
		double m[16];
		double rc[4][4];
	};
};
