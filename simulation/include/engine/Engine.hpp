#pragma once
#include <raylib.h>
#include <raymath.h>

#include <string>

struct v3 {
    float x, y, z;

    v3(float x, float y, float z);
    v3(Vector3 v);
    v3(void);              // starts a vector to {0, 0, 0}
    explicit v3(float v);  // vector with all values set to v

    v3 operator+(const v3& other);
    v3 operator-(const v3& other);
    v3 operator*(const v3& other);
    v3 operator/(const v3& other);
    v3 operator+=(const v3& other);
    v3 operator-=(const v3& other);
    v3 operator*=(const v3& other);
    v3 operator/=(const v3& other);

    v3 operator-(const float& other);
    v3 operator+(const float& other);
    v3 operator*(const float& other);
    v3 operator/(const float& other);
    v3 operator*=(const float& other);
    v3 operator/=(const float& other);

    std::string to_string(void) const;

    operator Vector3() const;
    operator float*() const;
};

struct Cell {
    v3 position;
    float density;
    bool is_solid;
    float distanceSquared;  // Distance from the camera (squared)
};

int constrain(int val, int low, int high);

void draw_text_codepoint_3d(int codepoint, Vector3 position, float font_size, Color color);
void draw_text_3d(std::string text, v3 position, float size, Color color);
