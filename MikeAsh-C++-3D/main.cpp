#include "Fluid.h"
#include <bits/stdc++.h>
#include <cstdint>
#include <raylib.h>
#include <raymath.h>

#define endl "\n"
#define WIDTH 640
#define HEIGHT 640

int main(int argc, char *argv[]) {
    srand(time(nullptr));
    InitWindow(WIDTH, HEIGHT, "Fluid Grid");
    float diffusion = 0;        // Diffusion constant
    float dt = 0.1;             // Timestep
    float viscosity = 0.000001; // Viscosity constant

    int N = 64; // Size of the fluid cell N*N
    int Scale = 10;
    Vector3 Scale3 = {(float)Scale, (float)Scale, (float)Scale};

    Fluid fluid = Fluid(N, diffusion, viscosity, dt);
    Camera3D camera = {
        .position = {10, 10, 10},
        .target = Vector3Zero(),
        .up = {0.0, 1.0, 0.0},
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE,
    };
    Vector3 index = {
        ((float)WIDTH - Scale) / 2,
        ((float)HEIGHT - Scale) / 2,
        0,
    };
    float amtX, amtY, amtZ;
    int mouseX, mouseY;
    while (!WindowShouldClose()) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            mouseX = GetMousePosition().x;
            mouseY = GetMousePosition().y;
            amtX = amtY = amtZ = rand() % 11 - 5;
            fluid.addDensity(mouseX / Scale, mouseY / Scale, 0, 100); // 0 is hardcoded
            fluid.addVelocity(mouseX / Scale, mouseY / Scale, 0, amtX, amtY,
                              amtZ); // 0 is hardcoded
        }

        ClearBackground({0, 0, 0, 255});
        fluid.step();
        BeginDrawing();
        BeginMode3D(camera);
        UpdateCamera(&camera, CAMERA_FREE);
        for (int i = 1; i < fluid.Size(); i++) {
            for (int j = 1; j < fluid.Size(); j++) {
                for (int k = 1; k < fluid.Size(); k++) {
                    float density = fluid.Density(k, j, i);
                    index.x = k * Scale;
                    index.y = j * Scale;
                    index.z = i * Scale;
                    BeginBlendMode(BLEND_ALPHA);
                    DrawCubeV(index, Scale3, {100, 255, 255, (std::uint8_t)density});
                    EndBlendMode();
                }
            }
        }
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
