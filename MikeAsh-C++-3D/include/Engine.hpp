#include <raymath.h>

#include <string>

class v3 {
   public:
    float x, y, z;

    v3(float x, float y, float z);
    v3(Vector3 v);
    v3();

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

    std::string to_string();
    const char* to_cstr();

    operator Vector3();
};
