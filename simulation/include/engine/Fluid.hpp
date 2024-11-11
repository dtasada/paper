#include <math.h>
#include <raylib.h>
#include <stdlib.h>

#include "Engine.hpp"

#define IX(x, y, z)                                                 \
    int(constrain((x), 0, N - 1) + (constrain((y), 0, N - 1) * N) + \
        (constrain((z), 0, N - 1) * N * N))

#define IXv(vec)                                                            \
    int(constrain((vec).x, 0, N - 1) + (constrain((vec).y, 0, N - 1) * N) + \
        (constrain((vec).z, 0, N - 1) * N * N))

class Fluid {
   private:
    static const int iter = 4;

    float container_size;
    float fluid_size;
    float dt;
    float diff;
    float visc;

    float *s;
    float *density;

    float *Vx;
    float *Vy;
    float *Vz;

    float *Vx0;
    float *Vy0;
    float *Vz0;

   public:
    Fluid(int container_size, float scale, float diffusion, float viscosity, float dt);
    ~Fluid();
    float Density(v3 position);
    float FluidSize();
    int ContainerSize();
    void add_density(v3 position, float amount);
    void add_velocity(v3 position, v3 amount);
    void renderGrid();
    void step();

    void diffuse(int b, float *x, float *x0, float diff);
    void project(float *velocX, float *velocY, float *velocZ, float *p, float *div);
    void advect(int b, float *d, float *d0, float *velocX, float *velocY, float *velocZ);
    void set_bnd(int bound, float *x);
    void lin_solve(int b, float *x, float *x0, float a, float c);
};
