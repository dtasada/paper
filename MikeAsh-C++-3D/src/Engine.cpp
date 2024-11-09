#include "../include/Engine.hpp"

#include <raymath.h>

#include <format>
#include <string>

v3::v3(float x, float y, float z) : x(x), y(y), z(z) {};
v3::v3(Vector3 v) : x(v.x), y(v.y), z(v.z) {};
v3::v3() : x(0), y(0), z(0) {};

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

v3::operator Vector3() { return {x, y, z}; }
v3::operator float*() { return new float[3]{x, y, z}; }

std::string v3::to_string() { return std::format("v3({:.1f}, {:.1f}, {:.1f})", x, y, z); }
const char* v3::to_cstr() { return to_string().c_str(); }
