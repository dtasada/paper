#include <iostream>
#include <math.h>
#include <raylib.h>
#include <vector>

#include "things.hpp"

#define AIR_PRESSURE                                                           \
  101325 // Air pressure at sea level: N/m^2 #define AIR_DENSITY 1.225 // Air
         // pressure at sea level: kg/m^3

#define LIFT(liftCoef, airDensity, airflowSpeed, wingArea, wingHeight)         \
  liftCoef *airDensity *pow(airflowSpeed, 2) * wingArea * 0.5
#define FORCE_1(dP, dt) dP *dt
#define FORCE_2(C, P_air, dt) (C * P_air) / (2 * dt)
#define FORCE_3(C, p, v, S) C *p *pow(v, 2) * S * 0.5
#define P_AIR(m, v) m *v
#define REYNOLDS(p, v, D, frictionCoef) (p * v * D) / frictionCoef

int main() {
  InitWindow(800, 600, "Hello there!");
  SetTargetFPS(60);

  Plane plane = Plane(1000, 40, 400, 1000, 1000, 1000);
  Model model = LoadModel("models/model_1.obj");

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(BLACK);

    DrawModel(model, {0.0, 0.0, 0.0}, 1.0, WHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
