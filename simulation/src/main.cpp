#include <bits/stdc++.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <algorithm>
#include <cstdio>

#include "../include/engine/Engine.hpp"
#include "../include/engine/Fluid.hpp"
#include "../include/engine/Profiler.hpp"

#define FPS 60.0f
#define VECTOROF(x) \
    v3 { (float)x, (float)x, (float)x }

int main(int argc, char* argv[]) {
    srand(time(nullptr));

    /* Raylib setup */
    InitWindow(1280, 720, "Fluid Grid");
    SetTargetFPS(FPS);
    SetExitKey(0);
    DisableCursor();
    // HideCursor();

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    rlDisableDepthTest();
    rlDisableBackfaceCulling();

    bool cursor = false;
    Camera3D camera = {
        .position = v3(10, 10, 10),
        .target = v3(-100, -100, -100),
        .up = {0.0, 1.0, 0.0},
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    /* Fluid sim setup */
    float diffusion = 0;         // Diffusion constant
    float dt = 1.0;              // Timestep
    float viscosity = 0.000001;  // Viscosity constant

    Fluid fluid(64, 0.5f, diffusion, viscosity, dt);
    v3 containerSize = VECTOROF(fluid.ContainerSize() * fluid.FluidSize());
    v3 containerCenter = containerSize * 0.5f;

    while (!WindowShouldClose()) {
        /* Handle input */
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            float x = 1, y = 1, z = 1;
            float amount = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 4 : 20;
            fluid.addDensity({x, y, z}, 100);

            v3 vel(4, -9.81, 4);
            fluid.addVelocity({x, y, z}, VECTOROF(amount));
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (cursor)
                DisableCursor();
            else
                EnableCursor();
            cursor = !cursor;
        }

        if (IsKeyPressed(KEY_F)) ToggleFullscreen();

        if (!cursor) UpdateCamera(&camera, CAMERA_FREE);

        /* Update sim */
        fluid.step();

        // Calculate distance to camera for each cube. This is important for rendering purposes
        std::vector<Cell> cells;
        for (float z = 0; z < fluid.ContainerSize(); z++) {
            for (float y = 0; y < fluid.ContainerSize(); y++) {
                for (float x = 0; x < fluid.ContainerSize(); x++) {
                    float density = fluid.Density({x, y, z});
                    if (density > 0.01f) {  // Skip cubes with very low density
                        v3 cubePosition = {x, y, z};
                        cubePosition = cubePosition * VECTOROF(fluid.FluidSize());
                        cubePosition = cubePosition + (fluid.FluidSize() / 2);

                        // Calculate the squared distance to the camera
                        float dx = cubePosition.x - camera.position.x;
                        float dy = cubePosition.y - camera.position.y;
                        float dz = cubePosition.z - camera.position.z;
                        float distanceSquared = dx * dx + dy * dy + dz * dz;

                        // Store cube data
                        cells.push_back({cubePosition, density, distanceSquared});
                    }
                }
            }
        }
        std::ranges::sort(cells, std::greater{}, &Cell::distanceSquared);

        // Begin Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);

        DrawCubeWiresV(containerCenter, containerSize, RED);

        // Render cubes
        for (const Cell& cell : cells) {
            float norm = std::min(cell.density / 100.0f, 1.0f);
            float hue = (1.0f - norm) * 0.66f * 360.0f;

            Color c = ColorFromHSV(hue, 1.0f, 1.0f);
            Color color = {c.r, c.g, c.b, (uint8_t)(norm * 255)};

            BeginBlendMode(BLEND_ALPHA);
            DrawCubeV(cell.position, VECTOROF(fluid.FluidSize()), color);
            EndBlendMode();
        }

        EndMode3D();

        DrawFPS(10, 10);
        DrawText(std::string("Position: ").append(v3(camera.position).to_string()).c_str(), 10, 30,
                 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    Profiler::printResults();
    return 0;
}
