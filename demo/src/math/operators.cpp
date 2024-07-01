// Project includes
#include "math/operators.h"

// System includes
#include <algorithm>

#pragma region float3
    float3 negate(const float3& v)
    {
        return float3({ -v.x, -v.y, -v.z});
    }

    float length(const float3& v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    float3 normalize(const float3& v)
    {
        float l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
        return float3({ v.x / l, v.y / l , v.z / l });
    }

    float3 cross(const float3& v0, const float3& v1)
    {
        return float3({v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x});
    }

    float dot(const float3& v0, const float3& v1)
    {
        return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
    }

    float3 lerp(const float3& v0, const float3& v1, float f)
    {
        float v_c = 1.0f - f;
        return v0 * v_c + v1 * f;
    }

    float3 max_zero(const float3& v)
    {
        return float3({ std::max(v.x, 0.0f), std::max(v.y, 0.0f), std::max(v.z, 0.0f) });
    }

    float3 operator+(const float3& v0, const float3& v1)
    {
        return float3({ v0.x + v1.x, v0.y + v1.y , v0.z + v1.z });
    }

    float3 operator-(const float3& v0, const float3& v1)
    {
        return float3({ v0.x - v1.x, v0.y - v1.y , v0.z - v1.z });
    }

    float3 operator*(const float3& v, float s)
    {
        return float3({ v.x * s, v.y * s, v.z * s });
    }

    float3 operator/(const float3& v, float s)
    {
        return float3({ v.x / s, v.y / s, v.z / s });
    }
#pragma endregion

#pragma region float4
    float4 negate(const float4& v)
    {
        return float4({ -v.x, -v.y, -v.z, -v.w });
    }

    float4 normalize(const float4& v)
    {
        float l = length(v);
        return float4({ v.x / l, v.y / l , v.z / l, v.w / l});
    }

    float length(const float4& v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    }

    float4 max_zero(const float4& v)
    {
        return float4({ std::max(v.x, 0.0f), std::max(v.y, 0.0f), std::max(v.z, 0.0f), std::max(v.w, 0.0f) });
    }

    float4 lerp(const float4& v0, const float4& v1, float f)
    {
        float v_c = 1.0f - f;
        return v0 * v_c + v1 * f;
    }

    float3& xyz(float4& q)
    {
        return (float3&)q.x;
    }

    const float3& xyz(const float4& q)
    {
        return (const float3&)q.x;
    }

    float4 operator+(const float4& v0, const float4& v1)
    {
        return float4({ v0.x + v1.x, v0.y + v1.y , v0.z + v1.z, v0.w + v1.w });
    }

    float4 operator-(const float4& v0, const float4& v1)
    {
        return float4({ v0.x - v1.x, v0.y - v1.y , v0.z - v1.z, v0.w - v1.w });
    }

    float4 operator*(const float4& v, float s)
    {
        return float4({ v.x * s, v.y * s, v.z * s, v.w * s });
    }

    float4 operator/(const float4& v, float s)
    {
        return float4({ v.x / s, v.y / s, v.z / s, v.w / s });
    }
#pragma endregion

#pragma region double3
    double length(const double3& v)
    {
        return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    double3 lerp(const double3& v0, const double3& v1, double f)
    {
        double v_c = 1.0 - f;
        return v0 * v_c + v1 * f;
    }

    double3 operator+(const double3& v0, const double3& v1)
    {
        return double3({ v0.x + v1.x, v0.y + v1.y , v0.z + v1.z });
    }

    double3 operator-(const double3& v0, const double3& v1)
    {
        return double3({ v0.x - v1.x, v0.y - v1.y , v0.z - v1.z });
    }

    double3 operator*(const double3& v, double s)
    {
        return double3({ v.x * s, v.y * s, v.z * s });
    }

    double3 operator/(const double3& v, double s)
    {
        return double3({ v.x / s, v.y / s, v.z / s });
    }
#pragma endregion

#pragma region double4
    double4 operator*(const double4& v, double s)
    {
        return double4({ v.x * s, v.y * s, v.z * s, v.w * s });
    }

    double4 operator/(const double4& v, double s)
    {
        return double4({ v.x / s, v.y / s, v.z / s, v.w / s });
    }
#pragma endregion

#pragma region uint3
    uint32_t at(const uint3& v, uint32_t idx)
    {
        const uint32_t* data = &v.x;
        return data[idx];
    }

    uint3 operator*(const uint3& v0, const uint3& v1)
    {
        return uint3({ v0.x * v1.x, v0.y * v1.y, v0.z * v1.z });
    }

    uint3 operator*(const uint3& v, uint32_t s)
    {
        return uint3({ v.x * s, v.y * s, v.z * s });
    }

    uint3 operator&(const uint3& v, uint32_t s)
    {
        return uint3({ v.x & s, v.y & s, v.z & s });
    }
#pragma endregion

#pragma region uint4
    uint32_t at(const uint4& v, uint32_t idx)
    {
        const uint32_t* data = &v.x;
        return data[idx];
    }

    uint32_t& at(uint4& v, uint32_t idx)
    {
        uint32_t* data = &v.x;
        return data[idx];
    }
#pragma endregion

#pragma region float3x3
    float3x3 transpose(const float3x3& m)
    {
        float3x3 result = m;
        std::swap(result.m[1], result.m[3]);
        std::swap(result.m[2], result.m[6]);
        std::swap(result.m[5], result.m[7]);
        return result;
    }

    // Changed to match the hlsl behavior
    float3x3 mul(const float3x3& _mat2, const float3x3& _mat1)
    {
        float3x3 result;
        result.m[0] = _mat1.m[0] * _mat2.m[0] + _mat1.m[1] * _mat2.m[3] + _mat1.m[2] * _mat2.m[6];
        result.m[1] = _mat1.m[0] * _mat2.m[1] + _mat1.m[1] * _mat2.m[4] + _mat1.m[2] * _mat2.m[7];
        result.m[2] = _mat1.m[0] * _mat2.m[2] + _mat1.m[1] * _mat2.m[5] + _mat1.m[2] * _mat2.m[8];

        result.m[3] = _mat1.m[3] * _mat2.m[0] + _mat1.m[4] * _mat2.m[3] + _mat1.m[5] * _mat2.m[6];
        result.m[4] = _mat1.m[3] * _mat2.m[1] + _mat1.m[4] * _mat2.m[4] + _mat1.m[5] * _mat2.m[7];
        result.m[5] = _mat1.m[3] * _mat2.m[2] + _mat1.m[4] * _mat2.m[5] + _mat1.m[5] * _mat2.m[8];

        result.m[6] = _mat1.m[6] * _mat2.m[0] + _mat1.m[7] * _mat2.m[3] + _mat1.m[8] * _mat2.m[6];
        result.m[7] = _mat1.m[6] * _mat2.m[1] + _mat1.m[7] * _mat2.m[4] + _mat1.m[8] * _mat2.m[7];
        result.m[8] = _mat1.m[6] * _mat2.m[2] + _mat1.m[7] * _mat2.m[5] + _mat1.m[8] * _mat2.m[8];
        return result;
    }
#pragma region end

#pragma region float4x4
    float4x4 identity_float4x4()
    {
        return float4x4({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 });
    }

    float4x4 projection_matrix(float fov, float nearZ, float farZ, float aspectRatio)
    {
        float sinFov = sinf(fov);
        float cosFov = cosf(fov);
        float Height = cosFov / sinFov;
        float Width = Height / aspectRatio;
        float fRange = farZ / (farZ - nearZ);

        float4x4 proj;
        proj.m[0] = Width;
        proj.m[1] = 0.0f;
        proj.m[2] = 0.0f;
        proj.m[3] = 0.0f;

        proj.m[4] = 0.0f;
        proj.m[5] = Height;
        proj.m[6] = 0.0f;
        proj.m[7] = 0.0f;

        proj.m[8] = 0.0f;
        proj.m[9] = 0.0f;
        proj.m[10] = fRange;
        proj.m[11] = 1.0f;

        proj.m[12] = 0.0f;
        proj.m[13] = 0.0f;
        proj.m[14] = -fRange * nearZ;
        proj.m[15] = 0.0f;
        return proj;
    }

    float4x4 zoom_matrix(const float2& scale, const float2& offset)
    {
        float4x4 proj;
        proj.m[0] = scale.x;
        proj.m[1] = 0.0;
        proj.m[2] = 0.0f;
        proj.m[3] = 0.0f;

        proj.m[4] = 0.0f;
        proj.m[5] = scale.y;
        proj.m[6] = 0.0f;
        proj.m[7] = 0.0f;

        proj.m[8] = 0.0f;
        proj.m[9] = 0.0f;
        proj.m[10] = 1.0f;
        proj.m[11] = 0.0f;

        proj.m[12] = offset.x;
        proj.m[13] = offset.y;
        proj.m[14] = 0.0f;
        proj.m[15] = 1.0f;
        return proj;
    }

    // Changed to match the hlsl behavior
    float4x4 mul(const float4x4& _mat2, const float4x4& _mat1)
    {
        float4x4 result;
        result.m[0] = _mat1.m[0] * _mat2.m[0] + _mat1.m[1] * _mat2.m[4] + _mat1.m[2] * _mat2.m[8] + _mat1.m[3] * _mat2.m[12];
        result.m[1] = _mat1.m[0] * _mat2.m[1] + _mat1.m[1] * _mat2.m[5] + _mat1.m[2] * _mat2.m[9] + _mat1.m[3] * _mat2.m[13];
        result.m[2] = _mat1.m[0] * _mat2.m[2] + _mat1.m[1] * _mat2.m[6] + _mat1.m[2] * _mat2.m[10] + _mat1.m[3] * _mat2.m[14];
        result.m[3] = _mat1.m[0] * _mat2.m[3] + _mat1.m[1] * _mat2.m[7] + _mat1.m[2] * _mat2.m[11] + _mat1.m[3] * _mat2.m[15];

        result.m[4] = _mat1.m[4] * _mat2.m[0] + _mat1.m[5] * _mat2.m[4] + _mat1.m[6] * _mat2.m[8] + _mat1.m[7] * _mat2.m[12];
        result.m[5] = _mat1.m[4] * _mat2.m[1] + _mat1.m[5] * _mat2.m[5] + _mat1.m[6] * _mat2.m[9] + _mat1.m[7] * _mat2.m[13];
        result.m[6] = _mat1.m[4] * _mat2.m[2] + _mat1.m[5] * _mat2.m[6] + _mat1.m[6] * _mat2.m[10] + _mat1.m[7] * _mat2.m[14];
        result.m[7] = _mat1.m[4] * _mat2.m[3] + _mat1.m[5] * _mat2.m[7] + _mat1.m[6] * _mat2.m[11] + _mat1.m[7] * _mat2.m[15];

        result.m[8] = _mat1.m[8] * _mat2.m[0] + _mat1.m[9] * _mat2.m[4] + _mat1.m[10] * _mat2.m[8] + _mat1.m[11] * _mat2.m[12];
        result.m[9] = _mat1.m[8] * _mat2.m[1] + _mat1.m[9] * _mat2.m[5] + _mat1.m[10] * _mat2.m[9] + _mat1.m[11] * _mat2.m[13];
        result.m[10] = _mat1.m[8] * _mat2.m[2] + _mat1.m[9] * _mat2.m[6] + _mat1.m[10] * _mat2.m[10] + _mat1.m[11] * _mat2.m[14];
        result.m[11] = _mat1.m[8] * _mat2.m[3] + _mat1.m[9] * _mat2.m[7] + _mat1.m[10] * _mat2.m[11] + _mat1.m[11] * _mat2.m[15];

        result.m[12] = _mat1.m[12] * _mat2.m[0] + _mat1.m[13] * _mat2.m[4] + _mat1.m[14] * _mat2.m[8] + _mat1.m[15] * _mat2.m[12];
        result.m[13] = _mat1.m[12] * _mat2.m[1] + _mat1.m[13] * _mat2.m[5] + _mat1.m[14] * _mat2.m[9] + _mat1.m[15] * _mat2.m[13];
        result.m[14] = _mat1.m[12] * _mat2.m[2] + _mat1.m[13] * _mat2.m[6] + _mat1.m[14] * _mat2.m[10] + _mat1.m[15] * _mat2.m[14];
        result.m[15] = _mat1.m[12] * _mat2.m[3] + _mat1.m[13] * _mat2.m[7] + _mat1.m[14] * _mat2.m[11] + _mat1.m[15] * _mat2.m[15];
        return result;
    }

    float4 mul(const float4x4& _mat, const float4& _vec)
    {
        float4 result;
        result.x = _mat.m[0] * _vec.x + _mat.m[1] * _vec.y + _mat.m[2] * _vec.z + _mat.m[3] * _vec.w;
        result.y = _mat.m[4] * _vec.x + _mat.m[5] * _vec.y + _mat.m[6] * _vec.z + _mat.m[7] * _vec.w;
        result.z = _mat.m[8] * _vec.x + _mat.m[9] * _vec.y + _mat.m[10] * _vec.z + _mat.m[11] * _vec.w;
        result.w = _mat.m[12] * _vec.x + _mat.m[13] * _vec.y + _mat.m[14] * _vec.z + _mat.m[15] * _vec.w;
        return result;
    }

    float3 mul(const float4x4& _mat, const float3& _vec)
    {
        float3 result;
        result.x = _mat.m[0] * _vec.x + _mat.m[1] * _vec.y + _mat.m[2] * _vec.z;
        result.y = _mat.m[4] * _vec.x + _mat.m[5] * _vec.y + _mat.m[6] * _vec.z;
        result.z = _mat.m[8] * _vec.x + _mat.m[9] * _vec.y + _mat.m[10] * _vec.z;
        return result;
    }

    float4x4 translation_matrix(const float3& translation)
    {
        return float4x4({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, translation.x, translation.y, translation.z, 1 });
    }

    float4x4 rotation_matrix_x(const float angle)
    {
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        return float4x4({ 1, 0, 0, 0,
            0, cosA, sinA, 0,
            0, -sinA, cosA, 0,
            0, 0, 0, 1 });
    }

    float4x4 rotation_matrix_y(const float angle)
    {
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        return float4x4({ cosA, 0, -sinA, 0,
            0, 1, 0, 0,
            sinA, 0, cosA, 0,
            0, 0, 0, 1 });
    }

    float4x4 rotation_matrix_z(const float angle)
    {
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        return float4x4({ cosA, sinA, 0, 0,
            -sinA, cosA, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1 });
    }

    float4x4 rotation_matrix_axis(const float angle, const float3& axis)
    {
        float c = cosf(angle);
        float s = sinf(angle);
        float oMinC = 1.0f - c;
        float x_y_ominc = axis.x * axis.y * oMinC;
        float x_z_ominc = axis.x * axis.z * oMinC;
        float y_z_ominc = axis.y * axis.z * oMinC;

        float4x4 m = float4x4({c + axis.x * axis.x * oMinC, x_y_ominc - axis.z * s, x_z_ominc + axis.y * s, 0.0f,
                       x_y_ominc + axis.z * s, c + axis.y * axis.y * oMinC, y_z_ominc - axis.x * s, 0.0f,
                        x_z_ominc - axis.y * s, y_z_ominc + axis.x * s, c + axis.z * axis.z * oMinC, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f });
        return transpose(m);
    }

    float4x4 look_at_matrix(const float3& eyePos, const float3& eyeDir, const float3& up)
    {
        float3 R2 = normalize(eyeDir);

        float3 R0 = cross(up, R2);
        R0 = normalize(R0);

        float3 R1 = cross(R2, R0);

        float3 NegEyePosition = negate(eyePos);

        float D0 = dot(R0, NegEyePosition);
        float D1 = dot(R1, NegEyePosition);
        float D2 = dot(R2, NegEyePosition);

        float4x4 M;
        M.m[0] = R0.x;
        M.m[1] = R0.y;
        M.m[2] = R0.z;
        M.m[3] = D0;

        M.m[4] = R1.x;
        M.m[5] = R1.y;
        M.m[6] = R1.z;
        M.m[7] = D1;

        M.m[8] = R2.x;
        M.m[9] = R2.y;
        M.m[10] = R2.z;
        M.m[11] = D2;

        M.m[12] = 0;
        M.m[13] = 0;
        M.m[14] = 0;
        M.m[15] = 1.0;

        M = transpose(M);

        return M;
    }

    float4x4 transpose(const float4x4& m)
    {
        float4x4 result = m;
        std::swap(result.m[1], result.m[4]);
        std::swap(result.m[2], result.m[8]);
        std::swap(result.m[3], result.m[12]);
        std::swap(result.m[6], result.m[9]);
        std::swap(result.m[7], result.m[13]);
        std::swap(result.m[11], result.m[14]);
        return result;
    }

    float4x4 inverse(const float4x4& mat)
    {
        float4x4 res;
        float det;

        res.m[0] = mat.m[5] * mat.m[10] * mat.m[15] -
            mat.m[5] * mat.m[11] * mat.m[14] -
            mat.m[9] * mat.m[6] * mat.m[15] +
            mat.m[9] * mat.m[7] * mat.m[14] +
            mat.m[13] * mat.m[6] * mat.m[11] -
            mat.m[13] * mat.m[7] * mat.m[10];

        res.m[4] = -mat.m[4] * mat.m[10] * mat.m[15] +
            mat.m[4] * mat.m[11] * mat.m[14] +
            mat.m[8] * mat.m[6] * mat.m[15] -
            mat.m[8] * mat.m[7] * mat.m[14] -
            mat.m[12] * mat.m[6] * mat.m[11] +
            mat.m[12] * mat.m[7] * mat.m[10];

        res.m[8] = mat.m[4] * mat.m[9] * mat.m[15] -
            mat.m[4] * mat.m[11] * mat.m[13] -
            mat.m[8] * mat.m[5] * mat.m[15] +
            mat.m[8] * mat.m[7] * mat.m[13] +
            mat.m[12] * mat.m[5] * mat.m[11] -
            mat.m[12] * mat.m[7] * mat.m[9];

        res.m[12] = -mat.m[4] * mat.m[9] * mat.m[14] +
            mat.m[4] * mat.m[10] * mat.m[13] +
            mat.m[8] * mat.m[5] * mat.m[14] -
            mat.m[8] * mat.m[6] * mat.m[13] -
            mat.m[12] * mat.m[5] * mat.m[10] +
            mat.m[12] * mat.m[6] * mat.m[9];

        res.m[1] = -mat.m[1] * mat.m[10] * mat.m[15] +
            mat.m[1] * mat.m[11] * mat.m[14] +
            mat.m[9] * mat.m[2] * mat.m[15] -
            mat.m[9] * mat.m[3] * mat.m[14] -
            mat.m[13] * mat.m[2] * mat.m[11] +
            mat.m[13] * mat.m[3] * mat.m[10];

        res.m[5] = mat.m[0] * mat.m[10] * mat.m[15] -
            mat.m[0] * mat.m[11] * mat.m[14] -
            mat.m[8] * mat.m[2] * mat.m[15] +
            mat.m[8] * mat.m[3] * mat.m[14] +
            mat.m[12] * mat.m[2] * mat.m[11] -
            mat.m[12] * mat.m[3] * mat.m[10];

        res.m[9] = -mat.m[0] * mat.m[9] * mat.m[15] +
            mat.m[0] * mat.m[11] * mat.m[13] +
            mat.m[8] * mat.m[1] * mat.m[15] -
            mat.m[8] * mat.m[3] * mat.m[13] -
            mat.m[12] * mat.m[1] * mat.m[11] +
            mat.m[12] * mat.m[3] * mat.m[9];

        res.m[13] = mat.m[0] * mat.m[9] * mat.m[14] -
            mat.m[0] * mat.m[10] * mat.m[13] -
            mat.m[8] * mat.m[1] * mat.m[14] +
            mat.m[8] * mat.m[2] * mat.m[13] +
            mat.m[12] * mat.m[1] * mat.m[10] -
            mat.m[12] * mat.m[2] * mat.m[9];

        res.m[2] = mat.m[1] * mat.m[6] * mat.m[15] -
            mat.m[1] * mat.m[7] * mat.m[14] -
            mat.m[5] * mat.m[2] * mat.m[15] +
            mat.m[5] * mat.m[3] * mat.m[14] +
            mat.m[13] * mat.m[2] * mat.m[7] -
            mat.m[13] * mat.m[3] * mat.m[6];

        res.m[6] = -mat.m[0] * mat.m[6] * mat.m[15] +
            mat.m[0] * mat.m[7] * mat.m[14] +
            mat.m[4] * mat.m[2] * mat.m[15] -
            mat.m[4] * mat.m[3] * mat.m[14] -
            mat.m[12] * mat.m[2] * mat.m[7] +
            mat.m[12] * mat.m[3] * mat.m[6];

        res.m[10] = mat.m[0] * mat.m[5] * mat.m[15] -
            mat.m[0] * mat.m[7] * mat.m[13] -
            mat.m[4] * mat.m[1] * mat.m[15] +
            mat.m[4] * mat.m[3] * mat.m[13] +
            mat.m[12] * mat.m[1] * mat.m[7] -
            mat.m[12] * mat.m[3] * mat.m[5];

        res.m[14] = -mat.m[0] * mat.m[5] * mat.m[14] +
            mat.m[0] * mat.m[6] * mat.m[13] +
            mat.m[4] * mat.m[1] * mat.m[14] -
            mat.m[4] * mat.m[2] * mat.m[13] -
            mat.m[12] * mat.m[1] * mat.m[6] +
            mat.m[12] * mat.m[2] * mat.m[5];

        res.m[3] = -mat.m[1] * mat.m[6] * mat.m[11] +
            mat.m[1] * mat.m[7] * mat.m[10] +
            mat.m[5] * mat.m[2] * mat.m[11] -
            mat.m[5] * mat.m[3] * mat.m[10] -
            mat.m[9] * mat.m[2] * mat.m[7] +
            mat.m[9] * mat.m[3] * mat.m[6];

        res.m[7] = mat.m[0] * mat.m[6] * mat.m[11] -
            mat.m[0] * mat.m[7] * mat.m[10] -
            mat.m[4] * mat.m[2] * mat.m[11] +
            mat.m[4] * mat.m[3] * mat.m[10] +
            mat.m[8] * mat.m[2] * mat.m[7] -
            mat.m[8] * mat.m[3] * mat.m[6];

        res.m[11] = -mat.m[0] * mat.m[5] * mat.m[11] +
            mat.m[0] * mat.m[7] * mat.m[9] +
            mat.m[4] * mat.m[1] * mat.m[11] -
            mat.m[4] * mat.m[3] * mat.m[9] -
            mat.m[8] * mat.m[1] * mat.m[7] +
            mat.m[8] * mat.m[3] * mat.m[5];

        res.m[15] = mat.m[0] * mat.m[5] * mat.m[10] -
            mat.m[0] * mat.m[6] * mat.m[9] -
            mat.m[4] * mat.m[1] * mat.m[10] +
            mat.m[4] * mat.m[2] * mat.m[9] +
            mat.m[8] * mat.m[1] * mat.m[6] -
            mat.m[8] * mat.m[2] * mat.m[5];

        // Evaluate the determinant
        det = 1.0f / (mat.m[0] * res.m[0] + mat.m[1] * res.m[4] + mat.m[2] * res.m[8] + mat.m[3] * res.m[12]);

        // Apply it to every elemnt of the matrix
        for (uint32_t i = 0; i < 16; i++)
            res.m[i] *= det;
    
        // Return the inverse matrix
        return res;
    }
#pragma endregion


#pragma region double4x4
    double4x4 identity_double4x4()
    {
        return double4x4({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 });
    }

    double4x4 zoom_matrix(const double2& scale, const double2& offset)
    {
        double4x4 proj;
        proj.m[0] = scale.x;
        proj.m[1] = 0.0;
        proj.m[2] = 0.0f;
        proj.m[3] = 0.0f;

        proj.m[4] = 0.0f;
        proj.m[5] = scale.y;
        proj.m[6] = 0.0f;
        proj.m[7] = 0.0f;

        proj.m[8] = 0.0f;
        proj.m[9] = 0.0f;
        proj.m[10] = 1.0f;
        proj.m[11] = 0.0f;

        proj.m[12] = offset.x;
        proj.m[13] = offset.y;
        proj.m[14] = 0.0f;
        proj.m[15] = 1.0f;
        return proj;
    }

    double4x4 mul(const double4x4& _mat2, const double4x4& _mat1)
    {
        double4x4 result;
        result.m[0] = _mat1.m[0] * _mat2.m[0] + _mat1.m[1] * _mat2.m[4] + _mat1.m[2] * _mat2.m[8] + _mat1.m[3] * _mat2.m[12];
        result.m[1] = _mat1.m[0] * _mat2.m[1] + _mat1.m[1] * _mat2.m[5] + _mat1.m[2] * _mat2.m[9] + _mat1.m[3] * _mat2.m[13];
        result.m[2] = _mat1.m[0] * _mat2.m[2] + _mat1.m[1] * _mat2.m[6] + _mat1.m[2] * _mat2.m[10] + _mat1.m[3] * _mat2.m[14];
        result.m[3] = _mat1.m[0] * _mat2.m[3] + _mat1.m[1] * _mat2.m[7] + _mat1.m[2] * _mat2.m[11] + _mat1.m[3] * _mat2.m[15];

        result.m[4] = _mat1.m[4] * _mat2.m[0] + _mat1.m[5] * _mat2.m[4] + _mat1.m[6] * _mat2.m[8] + _mat1.m[7] * _mat2.m[12];
        result.m[5] = _mat1.m[4] * _mat2.m[1] + _mat1.m[5] * _mat2.m[5] + _mat1.m[6] * _mat2.m[9] + _mat1.m[7] * _mat2.m[13];
        result.m[6] = _mat1.m[4] * _mat2.m[2] + _mat1.m[5] * _mat2.m[6] + _mat1.m[6] * _mat2.m[10] + _mat1.m[7] * _mat2.m[14];
        result.m[7] = _mat1.m[4] * _mat2.m[3] + _mat1.m[5] * _mat2.m[7] + _mat1.m[6] * _mat2.m[11] + _mat1.m[7] * _mat2.m[15];

        result.m[8] = _mat1.m[8] * _mat2.m[0] + _mat1.m[9] * _mat2.m[4] + _mat1.m[10] * _mat2.m[8] + _mat1.m[11] * _mat2.m[12];
        result.m[9] = _mat1.m[8] * _mat2.m[1] + _mat1.m[9] * _mat2.m[5] + _mat1.m[10] * _mat2.m[9] + _mat1.m[11] * _mat2.m[13];
        result.m[10] = _mat1.m[8] * _mat2.m[2] + _mat1.m[9] * _mat2.m[6] + _mat1.m[10] * _mat2.m[10] + _mat1.m[11] * _mat2.m[14];
        result.m[11] = _mat1.m[8] * _mat2.m[3] + _mat1.m[9] * _mat2.m[7] + _mat1.m[10] * _mat2.m[11] + _mat1.m[11] * _mat2.m[15];

        result.m[12] = _mat1.m[12] * _mat2.m[0] + _mat1.m[13] * _mat2.m[4] + _mat1.m[14] * _mat2.m[8] + _mat1.m[15] * _mat2.m[12];
        result.m[13] = _mat1.m[12] * _mat2.m[1] + _mat1.m[13] * _mat2.m[5] + _mat1.m[14] * _mat2.m[9] + _mat1.m[15] * _mat2.m[13];
        result.m[14] = _mat1.m[12] * _mat2.m[2] + _mat1.m[13] * _mat2.m[6] + _mat1.m[14] * _mat2.m[10] + _mat1.m[15] * _mat2.m[14];
        result.m[15] = _mat1.m[12] * _mat2.m[3] + _mat1.m[13] * _mat2.m[7] + _mat1.m[14] * _mat2.m[11] + _mat1.m[15] * _mat2.m[15];
        return result;
    }

    double4x4 convert_to_double(const float4x4& m)
    {
        double4x4 matDB;
        for (uint32_t e = 0; e < 16; ++e)
            matDB.m[e] = m.m[e];
        return matDB;
    }

    double4x4 projection_matrix(double fov, double nearZ, double farZ, double aspectRatio)
    {
        double sinFov = sin(fov);
        double cosFov = cos(fov);
        double Height = cosFov / sinFov;
        double Width = Height / aspectRatio;
        double fRange = farZ / (farZ - nearZ);

        double4x4 proj;
        proj.m[0] = Width;
        proj.m[1] = 0.0;
        proj.m[2] = 0.0;
        proj.m[3] = 0.0;

        proj.m[4] = 0.0;
        proj.m[5] = Height;
        proj.m[6] = 0.0;
        proj.m[7] = 0.0;

        proj.m[8] = 0.0;
        proj.m[9] = 0.0;
        proj.m[10] = fRange;
        proj.m[11] = 1.0;

        proj.m[12] = 0.0;
        proj.m[13] = 0.0;
        proj.m[14] = -fRange * nearZ;
        proj.m[15] = 0.0;
        return proj;
    }
#pragma endregion

#pragma region quaternion
    float4 slerp(const float4& a, const  float4& b, float t)
    {
        // if either input is zero, return the other.
        if (length(a) == 0.0)
        {
            if (length(b) == 0.0)
            {
                return { 0, 0, 0, 1 };
            }
            return b;
        }
        else if (length(b) == 0.0)
        {
            return a;
        }

        float cosHalfAngle = a.w * b.w + dot(xyz(a), xyz(b));

        float4 b_p = b;
        if (cosHalfAngle >= 1.0 || cosHalfAngle <= -1.0)
        {
            return a;
        }
        else if (cosHalfAngle < 0.0)
        {
            b_p = negate(b_p);
            cosHalfAngle = -cosHalfAngle;
        }

        float blendA;
        float blendB;
        if (cosHalfAngle < 0.99)
        {
            // do proper slerp for big angles
            float halfAngle = acosf(cosHalfAngle);
            float sinHalfAngle = sinf(halfAngle);
            float oneOverSinHalfAngle = 1.0f / sinHalfAngle;
            blendA = sinf(halfAngle * (1.0f - t)) * oneOverSinHalfAngle;
            blendB = sinf(halfAngle * t) * oneOverSinHalfAngle;
        }
        else
        {
            // do lerp if angle is really small.
            blendA = 1.0f - t;
            blendB = t;
        }

        float4 result = a * blendA + b * blendB;
        if (length(result) > 0.0f)
        {
            return normalize(result);
        }
        return { 0, 0, 0, 1 };
    }

    float4 mul(const float4& q0, const float4& q1)
    {
        const float3& q0_xyz = xyz(q0);
        const float3& q1_xyz = xyz(q1);
        float3 r = q1_xyz * q0.w + q0_xyz * q1.w + cross(q0_xyz, q1_xyz);
        float rw = q0.w * q1.w - dot(q0_xyz, q1_xyz);
        return { r.x, r.y, r.z, rw };
    }

    float4x4 quaternion_to_matrix(const float4& quat)
    {
        float4x4 m = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        float x = quat.x, y = quat.y, z = quat.z, w = quat.w;
        float x2 = x + x, y2 = y + y, z2 = z + z;
        float xx = x * x2, xy = x * y2, xz = x * z2;
        float yy = y * y2, yz = y * z2, zz = z * z2;
        float wx = w * x2, wy = w * y2, wz = w * z2;

        m.rc[0][0] = 1.0f - (yy + zz);
        m.rc[1][0] = xy - wz;
        m.rc[2][0] = xz + wy;

        m.rc[0][1] = xy + wz;
        m.rc[1][1] = 1.0f - (xx + zz);
        m.rc[2][1] = yz - wx;

        m.rc[0][2] = xz - wy;
        m.rc[1][2] = yz + wx;
        m.rc[2][2] = 1.0f - (xx + yy);

        m.rc[3][3] = 1.0;

        return m;
    }

    float4 matrix_to_quaternion(const float4x4& mat)
    {
        float tr = mat.rc[0][0] + mat.rc[1][1] + mat.rc[2][2];
        float4 q = { 0, 0, 0, 0 };
        if (tr > 0)
        {
            float s = sqrtf(tr + 1.0f) * 2; // S=4*qw 
            q.w = 0.25f * s;
            q.x = (mat.rc[1][2] - mat.rc[2][1]) / s;
            q.y = (mat.rc[2][0] - mat.rc[0][2]) / s;
            q.z = (mat.rc[0][1] - mat.rc[1][0]) / s;
        }
        else if ((mat.rc[0][0] > mat.rc[1][1]) && (mat.rc[0][0] > mat.rc[2][2]))
        {
            float s = sqrtf(1.0f + mat.rc[0][0] - mat.rc[1][1] - mat.rc[2][2]) * 2; // S=4*qx 
            q.w = (mat.rc[1][2] - mat.rc[2][1]) / s;
            q.x = 0.25f * s;
            q.y = (mat.rc[1][0] + mat.rc[0][1]) / s;
            q.z = (mat.rc[2][0] + mat.rc[0][2]) / s;
        }
        else if (mat.rc[1][1] > mat.rc[2][2])
        {
            float s = sqrtf(1.0f + mat.rc[1][1] - mat.rc[0][0] - mat.rc[2][2]) * 2; // S=4*qy
            q.w = (mat.rc[2][0] - mat.rc[0][2]) / s;
            q.x = (mat.rc[1][0] + mat.rc[0][1]) / s;
            q.y = 0.25f * s;
            q.z = (mat.rc[2][1] + mat.rc[1][2]) / s;
        }
        else
        {
            float s = sqrtf(1.0f + mat.rc[2][2] - mat.rc[0][0] - mat.rc[1][1]) * 2; // S=4*qz
            q.w = (mat.rc[0][1] - mat.rc[1][0]) / s;
            q.x = (mat.rc[2][0] + mat.rc[0][2]) / s;
            q.y = (mat.rc[2][1] + mat.rc[1][2]) / s;
            q.z = 0.25f * s;
        }
        return q;
    }
#pragma endregion

uint32_t find_msb(uint32_t x)
{
    uint32_t depth = 0;
    while (x > 0u) {
        ++depth;
        x >>= 1u;
    }
    return depth;
}

uint32_t find_msb_64(uint64_t x)
{
    uint32_t depth = 0;
    while (x > 0u) {
        ++depth;
        x >>= 1uLL;
    }
    return depth;
}

int32_t round_up_power2(uint32_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

uint32_t countbits(uint32_t i)
{
    i = i - ((i >> 1) & 0x55555555);        // add pairs of bits
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);  // quads
    i = (i + (i >> 4)) & 0x0F0F0F0F;        // groups of 8
    return (i * 0x01010101) >> 24;          // horizontal sum of bytes
}

uint32_t countbits(uint64_t i)
{
    return countbits(uint32_t(i & 0xffffffff)) + countbits(uint32_t((i >> 32) & 0xffffffff));
}