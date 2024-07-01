#pragma once

// Project includes
#include "math/types.h"

#pragma region float3
	float3 negate(const float3& v);
	float3 normalize(const float3& v);
	float length(const float3& v);
	float3 max_zero(const float3& v);
	float3 cross(const float3& v0, const float3& v1);
	float dot(const float3& v0, const float3& v1);
	float3 lerp(const float3& v0, const float3& v1, float f);
	float3 operator+(const float3& v0, const float3& v1);
	float3 operator-(const float3& v0, const float3& v1);
	float3 operator*(const float3& v, float s);
	float3 operator/(const float3& v, float s);
#pragma endregion

#pragma region float4
	float4 negate(const float4& v);
	float4 normalize(const float4& v);
	float length(const float4& v);
	float4 max_zero(const float4& v);
	float3& xyz(float4& q);
	const float3& xyz(const float4& q);
	float4 lerp(const float4& v0, const float4& v1, float f);
	float4 operator+(const float4& v0, const float4& v1);
	float4 operator-(const float4& v0, const float4& v1);
	float4 operator*(const float4& v, float s);
	float4 operator/(const float4& v, float s);
#pragma endregion

#pragma region double3
	double length(const double3& v);
	double3 lerp(const double3& v0, const double3& v1, double f);
	double3 operator+(const double3& v0, const double3& v1);
	double3 operator-(const double3& v0, const double3& v1);
	double3 operator*(const double3& v, double s);
	double3 operator/(const double3& v, double s);
#pragma endregion

#pragma region double4
	double4 operator*(const double4& v, double s);
	double4 operator/(const double4& v, double s);
#pragma endregion

#pragma region uint3
	uint32_t at(const uint3& v, uint32_t idx);
	uint3 operator*(const uint3& v0, const uint3& v1);
	uint3 operator*(const uint3& v, uint32_t s);
	uint3 operator&(const uint3& v, uint32_t s);
#pragma endregion

#pragma region uint4
	uint32_t at(const uint4& v, uint32_t idx);
	uint32_t& at(uint4& v, uint32_t idx);
#pragma endregion

#pragma region float3x3
	float3x3 transpose(const float3x3& m);
	float3x3 mul(const float3x3& m0, const float3x3& m1);
#pragma region end

#pragma region float4x4
	float4x4 identity_float4x4();
	float4x4 translation_matrix(const float3& translation);
	float4x4 rotation_matrix_x(const float angle);
	float4x4 rotation_matrix_y(const float angle);
	float4x4 rotation_matrix_z(const float angle);
	float4x4 rotation_matrix_axis(const float angle, const float3& axis);
	float4x4 projection_matrix(float fov, float nearZ, float farZ, float aspectRatio);
	float4x4 zoom_matrix(const float2& scale, const float2& offset);
	float4x4 look_at_matrix(const float3& eye, const float3& focus, const float3& up);
	float4x4 mul(const float4x4& m0, const float4x4& m1);
	float4x4 transpose(const float4x4& m);
	float4x4 inverse(const float4x4& m);
	float4 mul(const float4x4& m0, const float4& v);
	float3 mul(const float4x4& m0, const float3& v);
#pragma endregion

#pragma region double4x4
	double4x4 identity_double4x4();
	double4x4 zoom_matrix(const double2& scale, const double2& offset);
	double4x4 mul(const double4x4& m0, const double4x4& m1);
	double4x4 convert_to_double(const float4x4& m);
	double4x4 projection_matrix(double fov, double nearZ, double farZ, double aspectRatio);
#pragma endregion

#pragma region quaternion
	float4 slerp(const float4& a, const  float4& b, float t);
	float4 mul(const float4& q0, const float4& q1);
	float4x4 quaternion_to_matrix(const float4& quat);
	float4 matrix_to_quaternion(const float4x4& m);
#pragma endregion

// Utility functions
uint32_t find_msb(uint32_t x);
uint32_t find_msb_64(uint64_t x);
int32_t round_up_power2(uint32_t v);
uint32_t countbits(uint32_t i);
uint32_t countbits(uint64_t i);