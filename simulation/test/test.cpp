#include <raylib.h>

int main() {
    InitWindow(1280, 720, "test");
    SetTargetFPS(60);
    Camera3D camera = {
        .position = {2.0f, 2.0f, 2.0f},
        .target = {0.0f, 0.0f, 0.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    Model cube = LoadModel("cube.obj");

    while (!WindowShouldClose()) {
        BeginDrawing();
        BeginMode3D(camera);
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);

        ClearBackground(RAYWHITE);
        DrawGrid(10, 1.0f);
        DrawCubeV({0.5f, 0.5f, 0.5f}, {1, 1, 1}, RED);
        DrawModel(cube, {0.5f, 0.5f, 0.5f}, 2.0f, GREEN);

        EndMode3D();
        EndDrawing();
    }

    UnloadModel(cube);
    CloseWindow();
    return 0;
}
