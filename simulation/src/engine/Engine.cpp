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

bool check_collision_bounding_box(BoundingBox box, Vector3 point) {
    // Simple check if the point is within the bounding box
    return (point.x >= box.min.x && point.x <= box.max.x) &&
           (point.y >= box.min.y && point.y <= box.max.y) &&
           (point.z >= box.min.z && point.z <= box.max.z);
}

bool check_collision_mesh(Mesh mesh, Vector3 point) {
    // Get the bounding box of the mesh
    BoundingBox box = GetMeshBoundingBox(mesh);

    // First check: Is the point inside the mesh's bounding box?
    if (!check_collision_bounding_box(box, point)) {
        return false;
    }

    // Second check (optional): More complex, raycasting or point-in-triangle tests can be added
    // here

    return true;
}
