#pragma once
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include <array>
#include <vector>

#include "Engine.hpp"

#define IX(x, y, z)                                                    \
    std::clamp(int(x), 0, container_size - 1) +                        \
        (std::clamp(int(y), 0, container_size - 1) * container_size) + \
        (std::clamp(int(z), 0, container_size - 1) * container_size * container_size)

#define IXv(vec)                                                             \
    std::clamp(int((vec).x), 0, container_size - 1) +                        \
        (std::clamp(int((vec).y), 0, container_size - 1) * container_size) + \
        (std::clamp(int((vec).z), 0, container_size - 1) * container_size * container_size)

#define SIM_RES 24
#define FIELD_SIZE (SIM_RES * SIM_RES * SIM_RES)

#define N container_size

/** contains an array of size N^3 */
template <typename T>
using Field = std::array<T, FIELD_SIZE>;

enum class FieldType { VX, VY, VZ, DENSITY };
enum class CellType { SOLID, FLUID, CUT_CELL };

class Fluid {
   private:
    float dt;   /** simulation timestep */
    float visc; /** viscosity constant */

    /** 3D cell property fields */
    Field<float> s, density;    /** density fields */
    Field<float> vx, vy, vz;    /** velocity fields */
    Field<float> vx0, vy0, vz0; /** backup velocity fields */

    Field<CellType> state;  // cell state field
    std::vector<v3> boundary_cells;
    std::vector<float> boundary_cache;

    bool is_boundary_dirty = true;

   public:
    int container_size;
    float fluid_size;
    float diffusion;

    std::vector<Obstacle> obstacles;

    Fluid(int container_size, float fluid_size, float diffusion, float viscosity, float dt);
    ~Fluid(void);
    float get_density(v3 position);
    v3 get_velocity(v3 position);
    v3 get_position(int i);
    void reset(void);

    void add_density(v3 position, float amount);
    void add_velocity(v3 position, v3 amount);

    void advect(FieldType b, Field<float>& d, Field<float>& d0, Field<float>& velocX,
                Field<float>& velocY, Field<float>& velocZ);
    void diffuse(FieldType b, Field<float>& x, Field<float>& x0, float diff);
    void lin_solve(FieldType b, Field<float>& x, Field<float>& x0, float a, float c);
    void project(Field<float>& velocX, Field<float>& velocY, Field<float>& velocZ, Field<float>& p,
                 Field<float>& div);
    void set_boundaries(FieldType b, Field<float>& x);
    void step(void);

    // geometry
    void add_obstacle(v3 position, Model model);
    void voxelize(Obstacle& obstacle);
    float get_fractional_volume(v3 position);
    CellType get_state(v3 position);
};

bool point_in_box(v3 point, BoundingBox box);
