#include <math.h>
#include <raylib.h>
#include <stdlib.h>
#define IXv(vec)                                                     \
    (constrain(vec.x, 0, N - 1) + (constrain(vec.y, 0, N - 1) * N) + \
     (constrain(vec.z, 0, N - 1) * N * N))
#define IX(x, y, z) \
    (constrain(x, 0, N - 1) + (constrain(y, 0, N - 1) * N) + (constrain(z, 0, N - 1) * N * N))

#include "Engine.hpp"

class Fluid {
   private:
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
    void addDensity(v3 position, float amount);
    void addVelocity(v3 position, v3 amount);
    void renderGrid();
    void step();
};

void set_bnd(int bound, float *x, int N);
void lin_solve(int b, float *x, float *x0, float a, float c, int iter, int N);
void diffuse(int b, float *x, float *x0, float diff, float dt, int iter, int N);
void project(float *velocX, float *velocY, float *velocZ, float *p, float *div, int iter, int N);
void advect(int b, float *d, float *d0, float *velocX, float *velocY, float *velocZ, float dt,
            int N);
int constrain(int val, int low, int high);
