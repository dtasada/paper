#include <math.h>
#include <raylib.h>
#include <stdlib.h>

#include "Engine.hpp"

#define IX(x, y, z)                                                \
    int(constrain((x), 0, container_size - 1) +                    \
        (constrain((y), 0, container_size - 1) * container_size) + \
        (constrain((z), 0, container_size - 1) * container_size * container_size))

#define IXv(vec)                                                       \
    int(constrain((vec).x, 0, container_size - 1) +                    \
        (constrain((vec).y, 0, container_size - 1) * container_size) + \
        (constrain((vec).z, 0, container_size - 1) * container_size * container_size))

class Fluid {
   private:
    float dt;
    float visc;

    bool *solid;
    float *s;
    float *density;

    float *vx;
    float *vy;
    float *vz;

    float *vx0;
    float *vy0;
    float *vz0;

   public:
    int container_size;
    float fluid_size;
    float diffusion;

    Fluid(int container_size, float fluid_size, float diffusion, float viscosity, float dt);
    ~Fluid();
    float Density(v3 position);
    bool Solid(v3 position);
    void reset();
    v3 get_position(int i);

    void add_gravity();
    void add_density(v3 position, float amount);
    void add_velocity(v3 position, v3 amount);

    void advect(int b, float *d, float *d0, float *velocX, float *velocY, float *velocZ);
    void advect(float *d, float *d0, float *velocX, float *velocY, float *velocZ);
    void diffuse(int b, float *x, float *x0, float diff);
    void lin_solve(int b, float *x, float *x0, float a, float c);
    void project(float *velocX, float *velocY, float *velocZ, float *p, float *div);
    void set_boundaries(int bound, float *x);
    void set_boundaries(float *x);
    void step();

    // geometry
    void add_cube(v3 position, float size);
    void handle_solid_boundaries(float *x, int b);
};
