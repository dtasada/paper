#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include <array>
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

#define SIM_RES 24
#define FIELD_SIZE (SIM_RES * SIM_RES * SIM_RES)

#define N container_size

template <typename T>
using Field = std::array<T, FIELD_SIZE>;  // contains an array of size N*N*N

enum class FieldType { VX, VY, VZ, DENSITY };
enum class CellType { SOLID, FLUID, CUT_CELL };

class Fluid {
   private:
    float dt;    // simulation timestep
    float visc;  // viscosity constant

    // 3D cell property fields
    Field<float> s, density;     // density fields
    Field<float> vx, vy, vz;     // velocity fields
    Field<float> vx0, vy0, vz0;  // backup velocity fields

    Field<CellType> state;    // cell state field
    Field<float> solid_frac;  // solid fraction field

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
    void add_obstacle(v3 position, v3 size, Model model);
    void voxelize(Obstacle obstacle);
    CellType get_state(v3 position);
};
