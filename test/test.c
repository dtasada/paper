#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>

// Function to check ray-triangle intersection using Möller–Trumbore algorithm
bool CheckRayTriangleCollision(Ray ray, Vector3 v1, Vector3 v2, Vector3 v3,
                               Vector3 *outIntersection) {
    Vector3 edge1 = Vector3Subtract(v2, v1);
    Vector3 edge2 = Vector3Subtract(v3, v1);

    Vector3 h = Vector3CrossProduct(ray.direction, edge2);
    float a = Vector3DotProduct(edge1, h);

    if (a > -0.00001f && a < 0.00001f) {
        return false;  // Ray is parallel to the triangle
    }

    float f = 1.0f / a;
    Vector3 s = Vector3Subtract(ray.position, v1);
    float u = f * Vector3DotProduct(s, h);

    if (u < 0.0f || u > 1.0f) {
        return false;  // Intersection is outside the triangle
    }

    Vector3 q = Vector3CrossProduct(s, edge1);
    float v = f * Vector3DotProduct(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f) {
        return false;  // Intersection is outside the triangle
    }

    float t = f * Vector3DotProduct(edge2, q);
    if (t > 0.00001f) {  // Intersection is along the ray and in front of the ray origin
        if (outIntersection) {
            *outIntersection = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
        }
        return true;
    }

    return false;  // No valid intersection
}

// Function to check if a point is inside a model
bool IsPointInsideModel(Vector3 point, Mesh mesh) {
    int intersections = 0;

    // Cast mesh data
    float *vertices = (float *)mesh.vertices;
    unsigned short *indices = mesh.indices;  // Check if indices exist

    Ray ray = {point, {1.0f, 0.0f, 0.0f}};  // Ray shooting along +X

    for (int i = 0; i < mesh.triangleCount; i++) {
        Vector3 v1, v2, v3;

        // Extract triangle vertices
        if (indices) {
            v1 = (Vector3){vertices[indices[i * 3] * 3], vertices[indices[i * 3] * 3 + 1],
                           vertices[indices[i * 3] * 3 + 2]};
            v2 = (Vector3){vertices[indices[i * 3 + 1] * 3], vertices[indices[i * 3 + 1] * 3 + 1],
                           vertices[indices[i * 3 + 1] * 3 + 2]};
            v3 = (Vector3){vertices[indices[i * 3 + 2] * 3], vertices[indices[i * 3 + 2] * 3 + 1],
                           vertices[indices[i * 3 + 2] * 3 + 2]};
        } else {
            // If no indices, use triangle list
            v1 = (Vector3){vertices[i * 9], vertices[i * 9 + 1], vertices[i * 9 + 2]};
            v2 = (Vector3){vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5]};
            v3 = (Vector3){vertices[i * 9 + 6], vertices[i * 9 + 7], vertices[i * 9 + 8]};
        }

        // Check for ray-triangle collision
        Vector3 intersection;
        if (CheckRayTriangleCollision(ray, v1, v2, v3, &intersection)) {
            intersections++;
        }
    }

    // Odd intersections mean the point is inside
    return (intersections % 2) == 1;
}

// Classify cell relative to a concave model
const char *ClassifyCellConcave(Vector3 cellPos, Vector3 cellSize, Mesh mesh) {
    Vector3 corners[8] = {
        cellPos,
        {cellPos.x + cellSize.x, cellPos.y, cellPos.z},
        {cellPos.x, cellPos.y + cellSize.y, cellPos.z},
        {cellPos.x, cellPos.y, cellPos.z + cellSize.z},
        {cellPos.x + cellSize.x, cellPos.y + cellSize.y, cellPos.z},
        {cellPos.x, cellPos.y + cellSize.y, cellPos.z + cellSize.z},
        {cellPos.x + cellSize.x, cellPos.y, cellPos.z + cellSize.z},
        {cellPos.x + cellSize.x, cellPos.y + cellSize.y, cellPos.z + cellSize.z}};

    int insideCount = 0;

    // Check each corner
    for (int i = 0; i < 8; i++) {
        if (IsPointInsideModel(corners[i], mesh)) {
            insideCount++;
        }
    }

    if (insideCount == 8) {
        return "Inside";
    } else if (insideCount > 0) {
        return "Half-Inside";
    }

    return "Outside";
}

// Define some constants for the torus' radius
#define TORUS_RADIUS 5.0f
#define TORUS_TUBE_RADIUS 1.0f  // The radius of the tube that forms the torus

// Function to check if a point is inside the torus (given a position and radius)
bool IsPointInsideTorus(Vector3 point, Vector3 torusCenter) {
    // Calculate the distance from the point to the torus center
    float dx = point.x - torusCenter.x;
    float dz = point.z - torusCenter.z;
    float distance = sqrtf(dx * dx + dz * dz);

    // The point is inside the torus if it is within the torus' outer and inner radii
    return (distance >= TORUS_RADIUS - TORUS_TUBE_RADIUS &&
            distance <= TORUS_RADIUS + TORUS_TUBE_RADIUS);
}

// Function to check if the cell is inside the torus
bool IsCellInsideTorus(Vector3 cellPosition, Vector3 cellSize, Vector3 torusCenter) {
    // We will check multiple points within the cell's bounding box
    Vector3 min = Vector3Subtract(cellPosition, Vector3Scale(cellSize, 0.5f));  // Min corner
    Vector3 max = Vector3Add(cellPosition, Vector3Scale(cellSize, 0.5f));       // Max corner

    // Check the center of the cell first
    if (IsPointInsideTorus(cellPosition, torusCenter)) {
        return true;
    }

    // Check points around the cell (we're doing a simple test here, checking a few points)
    for (float x = min.x; x <= max.x; x += cellSize.x) {
        for (float y = min.y; y <= max.y; y += cellSize.y) {
            for (float z = min.z; z <= max.z; z += cellSize.z) {
                Vector3 testPoint = {x, y, z};
                if (IsPointInsideTorus(testPoint, torusCenter)) {
                    return true;  // If any point inside the cell is inside the torus, return true
                }
            }
        }
    }

    // If no points are inside the torus, return false
    return false;
}

int main(void) {
    // Initialize the Raylib window
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Torus and Cell Visualization");
    SetTargetFPS(60);
    DisableCursor();
    HideCursor();

    // Load the torus model
    Model torus = LoadModel("torus.obj");  // Replace with the actual path

    Texture2D texture =
        LoadTexture("resources/torus_texture.png");  // Optional, replace with actual path
    torus.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // Define the position of the torus in the world
    Vector3 torusPosition = {0.0f, 1.0f, 0.0f};  // Position of the torus in world coordinates

    // Define the cell
    Vector3 cellPosition = {0.0f, 1.0f, 0.0f};  // Center position of the cell
    Vector3 cellSize = {2.0f, 2.0f, 2.0f};      // Size of the cell

    // Define a camera
    Camera camera = {0};
    camera.position = (Vector3){10.0f, 10.0f, 10.0f};  // Camera position
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};       // Camera looking at the origin
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};           // Up vector
    camera.fovy = 45.0f;                               // Field of view
    camera.projection = CAMERA_PERSPECTIVE;            // Perspective camera

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        UpdateCamera(&camera, CAMERA_FREE);

        // Check if the cell is inside the torus
        bool isCellInsideTorus = IsCellInsideTorus(cellPosition, cellSize, torusPosition);

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // Draw the torus model at the defined position
        if (torus.meshCount > 0) {
            DrawModel(torus, torusPosition, 10.0f,
                      WHITE);  // Position the torus at `torusPosition`
        }

        // Draw the cell as a wireframe box
        Vector3 cellMin = Vector3Subtract(cellPosition, Vector3Scale(cellSize, 0.5f));
        Vector3 cellMax = Vector3Add(cellPosition, Vector3Scale(cellSize, 0.5f));
        DrawBoundingBox((BoundingBox){cellMin, cellMax}, RED);

        // Display if the cell is inside the torus
        if (isCellInsideTorus) {
            DrawText("Cell is inside the torus!", 10, 40, 20, GREEN);
        } else {
            DrawText("Cell is outside the torus!", 10, 40, 20, RED);
        }

        EndMode3D();

        DrawText("Torus and Cell Visualization", 10, 10, 20, DARKGRAY);
        DrawFPS(10, 40);

        EndDrawing();
    }

    // Cleanup
    UnloadModel(torus);      // Unload the torus model
    UnloadTexture(texture);  // Unload the texture
    CloseWindow();           // Close window and OpenGL context

    return 0;
}
