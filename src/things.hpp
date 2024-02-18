#include <vector>
#include <iostream>

struct Vertex {
	float x, y, z;
};

struct Plane {
	std::vector<Vertex> polygon;
	
    // Physical properties
    double mass;
    double wingspan;
    double wingArea;
    double dragCoef;
    double liftCoef;
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
		this->dragCoef = dragCoef; // unitless
		this->liftCoef = liftCoef; // unitless
		this->thrust = thrust; // N

		this->polygon = {
			{ -1.0f, -1.0f, -1.0f },
			{  1.0f, -1.0f, -1.0f },
			{  1.0f,  1.0f, -1.0f },
			{ -1.0f,  1.0f, -1.0f },
			{ -1.0f, -1.0f,  1.0f },
			{  1.0f, -1.0f,  1.0f },
			{  1.0f,  1.0f,  1.0f },
			{ -1.0f,  1.0f,  1.0f } 
		};
	}

    // Function to update the state of the airplane based on physics calculations
    void update(double timeStep) {
        // Implement physics calculations here
        // Update velocity, position, forces, and angles based on aerodynamics
        // ...

        std::cout << "Updating airplane state...\n";
    }
};

