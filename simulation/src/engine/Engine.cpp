#include "../../include/engine/Engine.hpp"

#include <raymath.h>

#include <format>
#include <string>

v3::v3(float x, float y, float z) : x(x), y(y), z(z) {};
v3::v3(float v) : x(v), y(v), z(v) {};
v3::v3(Vector3 v) : x(v.x), y(v.y), z(v.z) {};
v3::v3() : v3(0) {};

v3 v3::operator+(const v3& other) { return v3(x + other.x, y + other.y, z + other.z); }
v3 v3::operator-(const v3& other) { return v3(x - other.x, y - other.y, z - other.z); }
v3 v3::operator*(const v3& other) { return v3(x * other.x, y * other.y, z * other.z); }
v3 v3::operator/(const v3& other) { return v3(x / other.x, y / other.y, z / other.z); }

v3 v3::operator+=(const v3& other) { return *this = *this + other; }
v3 v3::operator-=(const v3& other) { return *this = *this - other; }
v3 v3::operator*=(const v3& other) { return *this = *this * other; }
v3 v3::operator/=(const v3& other) { return *this = *this / other; }

v3 v3::operator-(const float& other) { return v3(x - other, y - other, z - other); }
v3 v3::operator+(const float& other) { return v3(x + other, y + other, z + other); }
v3 v3::operator*(const float& other) { return v3(x * other, y * other, z * other); }
v3 v3::operator/(const float& other) { return v3(x / other, y / other, z / other); }
v3 v3::operator*=(const float& other) { return *this = *this * other; }
v3 v3::operator/=(const float& other) { return *this = *this / other; }

v3::operator Vector3() const { return {x, y, z}; }
v3::operator float*() const { return new float[3]{x, y, z}; }

std::string v3::to_string() const { return std::format("v3({:.1f}, {:.1f}, {:.1f})", x, y, z); }

int constrain(int val, int low, int high) { return std::min(std::max(val, low), high); }
