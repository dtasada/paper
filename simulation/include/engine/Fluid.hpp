#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include <vector>

#include "Engine.hpp"

#define IX(x, y, z)                                                \
    int(constrain((x), 0, container_size - 1) +                    \
        (constrain((y), 0, container_size - 1) * container_size) + \
        (constrain((z), 0, container_size - 1) * container_size * container_size))

#define IXv(vec)                                                       \
    int(constrain((vec).x, 0, container_size - 1) +                    \
        (constrain((vec).y, 0, container_size - 1) * container_size) + \
        (constrain((vec).z, 0, container_size - 1) * container_size * container_size))

enum class FieldType { VX, VY, VZ, DENSITY };
enum class CellType { SOLID, FLUID, CUT_CELL };

class Fluid {
   private:
    float dt;    // simulation timestep
    float visc;  // viscosity constant

    // 3D cell property fields
    bool *solid;             // binary solid geometry
    float *s, *density;      // density fields
    float *vx, *vy, *vz;     // velocity fields
    float *vx0, *vy0, *vz0;  // backup velocity fields
                             /// float *solid_frac; // not implemented

    // std::vector<Obstacle> obstacles; // not implemented

   public:
    int container_size;
    float fluid_size;
    float diffusion;

    Fluid(int container_size, float fluid_size, float diffusion, float viscosity, float dt);
    ~Fluid(void);
    float get_density(v3 position);
    v3 get_velocity(v3 position);
    v3 get_position(int i);
    bool is_solid(v3 position);
    void set_solid(v3 position, bool set);
    void reset(void);

    void add_density(v3 position, float amount);
    void add_velocity(v3 position, v3 amount);

    void advect(FieldType b, float *d, float *d0, float *velocX, float *velocY, float *velocZ);
    void diffuse(FieldType b, float *x, float *x0, float diff);
    void lin_solve(FieldType b, float *x, float *x0, float a, float c);
    void project(float *velocX, float *velocY, float *velocZ, float *p, float *div);
    void set_boundaries(FieldType b, float *x);
    void step(void);

    // geometry
    void add_cube(v3 position, float size);
    void handle_solid_boundaries(float *x, int b);
    /* not implemented
void add_obstacle(v3 position, Model model);
void voxelize_obstacles(void);
    */

    void print_density(void);
};
