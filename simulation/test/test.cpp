#include <raylib.h>

int main() {
    InitWindow(1280, 720, "test");
    SetTargetFPS(60);
    Camera3D camera = {
        .position = {2.0f, 2.0f, 2.0f},
        .target = {0.0f, 0.0f, 0.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    Model cube = LoadModel("cube.obj");
    Model plane = LoadModel("plane.obj");

    while (!WindowShouldClose()) {
        BeginDrawing();
        BeginMode3D(camera);
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);

        ClearBackground(RAYWHITE);
        DrawGrid(10, 1.0f);
        DrawModel(cube, {0, 0, 0}, 1.0f, GREEN);
        DrawModel(plane, {4, 4, 4}, 1.0f, GREEN);

        EndMode3D();
        EndDrawing();
    }

    UnloadModel(cube);
    UnloadModel(plane);
    CloseWindow();
    return 0;
}
