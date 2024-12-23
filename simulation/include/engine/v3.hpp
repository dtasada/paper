#pragma once
#include <fcl/common/types.h>
#include <raylib.h>

#include <string>

struct v3 {
    float x, y, z;

    v3(float x, float y, float z);
    v3(Vector3 v);
    v3(void);              // starts a vector to {0, 0, 0}
    explicit v3(float v);  // vector with all values set to v

    v3 operator+(const v3& other) const;
    v3 operator-(const v3& other) const;
    v3 operator*(const v3& other) const;
    v3 operator/(const v3& other) const;

    v3 operator+=(const v3& other);
    v3 operator-=(const v3& other);
    v3 operator*=(const v3& other);
    v3 operator/=(const v3& other);

    v3 operator-(const float& other) const;
    v3 operator+(const float& other) const;
    v3 operator*(const float& other) const;
    v3 operator/(const float& other) const;

    v3 operator*=(const float& other);
    v3 operator/=(const float& other);
    v3 operator+=(const float& other);
    v3 operator-=(const float& other);

    std::string to_string(void) const;

    operator Vector3() const;
    operator fcl::Vector3f() const;
    explicit operator float*() const;
};
