// discord.gg/thenwefuckin
#pragma once
#include <cmath>
#include <algorithm>
#undef min
#undef max

namespace Mesh {

struct Vector3
{
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3 operator/(float scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }
    Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vector3& operator-=(const Vector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    Vector3& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    Vector3& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }
    Vector3 operator-() const { return Vector3(-x, -y, -z); }

    bool operator==(const Vector3& other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const Vector3& other) const { return !(*this == other); }

    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    float LengthSquared() const { return x * x + y * y + z * z; }

    Vector3 Normalized() const
    {
        float len = Length();
        if (len > 0.f)
        {
            return *this / len;
        }

        return Vector3();
    }

    void Normalize()
    {
        float len = Length();
        if (len > 0.f)
        {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    float Dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
    Vector3 Cross(const Vector3& other) const
    {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    float DistanceTo(const Vector3& other) const { return (*this - other).Length(); }
    float DistanceSquaredTo(const Vector3& other) const { return (*this - other).LengthSquared(); }

    static Vector3 Zero() { return Vector3(0, 0, 0); }
    static Vector3 One() { return Vector3(1, 1, 1); }
    static Vector3 Up() { return Vector3(0, 1, 0); }
    static Vector3 Down() { return Vector3(0, -1, 0); }
    static Vector3 Left() { return Vector3(-1, 0, 0); }
    static Vector3 Right() { return Vector3(1, 0, 0); }
    static Vector3 Forward() { return Vector3(0, 0, 1); }
    static Vector3 Back() { return Vector3(0, 0, -1); }

    static Vector3 Lerp(const Vector3& a, const Vector3& b, float t) { return a + (b - a) * t; }
};

struct Vector2
{
    float x, y;

    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}

    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
    Vector2 operator/(float scalar) const { return Vector2(x / scalar, y / scalar); }
    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
    Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
    Vector2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vector2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }
    Vector2 operator-() const { return Vector2(-x, -y); }

    bool operator==(const Vector2& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2& other) const { return !(*this == other); }

    float Length() const { return std::sqrt(x * x + y * y); }
    float LengthSquared() const { return x * x + y * y; }

    Vector2 Normalized() const
    {
        float len = Length();
        if (len > 0.f)
        {
            return *this / len;
        }

        return Vector2();
    }

    void Normalize()
    {
        float len = Length();
        if (len > 0.f)
        {
            x /= len;
            y /= len;
        }
    }

    float Dot(const Vector2& other) const { return x * other.x + y * other.y; }
    float Cross(const Vector2& other) const { return x * other.y - y * other.x; }

    float DistanceTo(const Vector2& other) const { return (*this - other).Length(); }
    float DistanceSquaredTo(const Vector2& other) const { return (*this - other).LengthSquared(); }

    static Vector2 Zero() { return Vector2(0, 0); }
    static Vector2 One() { return Vector2(1, 1); }
    static Vector2 Up() { return Vector2(0, 1); }
    static Vector2 Down() { return Vector2(0, -1); }
    static Vector2 Left() { return Vector2(-1, 0); }
    static Vector2 Right() { return Vector2(1, 0); }

    static Vector2 Lerp(const Vector2& a, const Vector2& b, float t) { return a + (b - a) * t; }
};

struct Color3
{
    float r, g, b;

    Color3() : r(0), g(0), b(0) {}
    Color3(float r, float g, float b) : r(r), g(g), b(b) {}

    Color3 operator+(const Color3& other) const { return Color3(r + other.r, g + other.g, b + other.b); }
    Color3 operator-(const Color3& other) const { return Color3(r - other.r, g - other.g, b - other.b); }
    Color3 operator*(float scalar) const { return Color3(r * scalar, g * scalar, b * scalar); }
    Color3 operator*(const Color3& other) const { return Color3(r * other.r, g * other.g, b * other.b); }
    Color3 operator/(float scalar) const { return Color3(r / scalar, g / scalar, b / scalar); }
    Color3& operator+=(const Color3& other) { r += other.r; g += other.g; b += other.b; return *this; }
    Color3& operator-=(const Color3& other) { r -= other.r; g -= other.g; b -= other.b; return *this; }
    Color3& operator*=(float scalar) { r *= scalar; g *= scalar; b *= scalar; return *this; }
    Color3& operator/=(float scalar) { r /= scalar; g /= scalar; b /= scalar; return *this; }

    bool operator==(const Color3& other) const { return r == other.r && g == other.g && b == other.b; }
    bool operator!=(const Color3& other) const { return !(*this == other); }

    Color3 Clamp() const
    {
        float rr = r;
        float gg = g;
        float bb = b;
        if (rr < 0.f) rr = 0.f;
        if (rr > 1.f) rr = 1.f;
        if (gg < 0.f) gg = 0.f;
        if (gg > 1.f) gg = 1.f;
        if (bb < 0.f) bb = 0.f;
        if (bb > 1.f) bb = 1.f;
        return Color3(rr, gg, bb);
    }

    static Color3 Black() { return Color3(0, 0, 0); }
    static Color3 White() { return Color3(1, 1, 1); }
    static Color3 Red() { return Color3(1, 0, 0); }
    static Color3 Green() { return Color3(0, 1, 0); }
    static Color3 Blue() { return Color3(0, 0, 1); }
    static Color3 Yellow() { return Color3(1, 1, 0); }
    static Color3 Cyan() { return Color3(0, 1, 1); }
    static Color3 Magenta() { return Color3(1, 0, 1); }

    static Color3 Lerp(const Color3& a, const Color3& b, float t) { return a + (b - a) * t; }
};

struct Matrix4x4
{
    float m[4][4];

    Matrix4x4()
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (i == j)
                    m[i][j] = 1.f;
                else
                    m[i][j] = 0.f;
            }
        }
    }

    Matrix4x4(float m00, float m01, float m02, float m03,
              float m10, float m11, float m12, float m13,
              float m20, float m21, float m22, float m23,
              float m30, float m31, float m32, float m33)
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }

    Matrix4x4 operator*(const Matrix4x4& other) const
    {
        Matrix4x4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result.m[i][j] = 0;
                for (int k = 0; k < 4; k++)
                    result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
        return result;
    }

    Vector3 MultiplyPoint(const Vector3& point) const
    {
        float x = m[0][0] * point.x + m[0][1] * point.y + m[0][2] * point.z + m[0][3];
        float y = m[1][0] * point.x + m[1][1] * point.y + m[1][2] * point.z + m[1][3];
        float z = m[2][0] * point.x + m[2][1] * point.y + m[2][2] * point.z + m[2][3];
        return Vector3(x, y, z);
    }

    Vector3 MultiplyVector(const Vector3& vector) const
    {
        float x = m[0][0] * vector.x + m[0][1] * vector.y + m[0][2] * vector.z;
        float y = m[1][0] * vector.x + m[1][1] * vector.y + m[1][2] * vector.z;
        float z = m[2][0] * vector.x + m[2][1] * vector.y + m[2][2] * vector.z;
        return Vector3(x, y, z);
    }

    static Matrix4x4 Identity()
    {
        return Matrix4x4();
    }

    static Matrix4x4 Translation(float x, float y, float z)
    {
        Matrix4x4 result;
        result.m[0][3] = x;
        result.m[1][3] = y;
        result.m[2][3] = z;
        return result;
    }

    static Matrix4x4 Translation(const Vector3& translation)
    {
        return Translation(translation.x, translation.y, translation.z);
    }

    static Matrix4x4 RotationX(float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix4x4 result;
        result.m[1][1] = c;
        result.m[1][2] = -s;
        result.m[2][1] = s;
        result.m[2][2] = c;
        return result;
    }

    static Matrix4x4 RotationY(float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix4x4 result;
        result.m[0][0] = c;
        result.m[0][2] = s;
        result.m[2][0] = -s;
        result.m[2][2] = c;
        return result;
    }

    static Matrix4x4 RotationZ(float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix4x4 result;
        result.m[0][0] = c;
        result.m[0][1] = -s;
        result.m[1][0] = s;
        result.m[1][1] = c;
        return result;
    }

    static Matrix4x4 Scale(float x, float y, float z)
    {
        Matrix4x4 result;
        result.m[0][0] = x;
        result.m[1][1] = y;
        result.m[2][2] = z;
        return result;
    }

    static Matrix4x4 Scale(const Vector3& scale)
    {
        return Scale(scale.x, scale.y, scale.z);
    }

    static Matrix4x4 Scale(float uniformScale)
    {
        return Scale(uniformScale, uniformScale, uniformScale);
    }
};

}
