#include <iostream>
#include <raylib.h>
#include <math.h>

#define AIR_PRESSURE 101325 // Air pressure at sea level: N/m^2
#define AIR_DENSITY 1.225 // Air pressure at sea level: kg/m^3

#define LIFT(liftCoef, airDensity, airflowSpeed, wingArea, wingHeight) \
	liftCoef * airDensity * pow(airflowSpeed, 2) * wingArea * 0.5
#define FORCE_1(dP, dt) dP * dt
#define FORCE_2(C, P_air, dt) (C * P_air) / (2 * dt)
#define FORCE_3(C, p, v, S) C * p * pow(v, 2) * S * 0.5
#define P_AIR(m, v) m * v
#define REYNOLDS(p, v, D, frictionCoef) (p * v * D) / frictionCoef

struct Plane {
    // Physical properties
    double mass;
    double wingspan;
    double wingArea;
    double dragCoefficient;
    double liftCoefficient;
    double thrust;

    // State variables
	double velocity;
	double altitude;
	double pitchAngle;
	double rollAngle;
	double yawAngle;

    // Aerodynamic forces and moments
    double liftForce;
    double dragForce;
    double thrustForce;
    double gravityForce;

    // Constructor
	Plane(double mass, double wingspan, double wingArea, double dragCoef, double liftCoef, double thrust) {
		this->mass = mass; // kg
		this->wingspan = wingspan; // m
		this->wingArea = wingArea; // m^2
		this->dragCoefficient = dragCoef; // unitless
		this->liftCoefficient = liftCoef; // unitless
		this->thrust = thrust; // N
	}

    // Function to update the state of the airplane based on physics calculations
    void update(double timeStep) {
        // Implement physics calculations here
        // Update velocity, position, forces, and angles based on aerodynamics
        // ...

        std::cout << "Updating airplane state...\n";
    }
};

int main() {
	InitWindow(800, 600, "Hello there!");

	Camera3D camera;
	Plane plane = Plane(1000, 40, 400, 1000, 1000, 1000);

	while (!WindowShouldClose()) {
		BeginMode3D(camera);
		BeginDrawing();

		EndDrawing();
		EndMode3D();
	}

	return 0;
}
