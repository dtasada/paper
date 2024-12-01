#include "../../include/engine/Fluid.hpp"

#include <raylib.h>
#include <raymath.h>

#include <cstring>
#include <vector>

#include "../../include/engine/Engine.hpp"

Fluid::Fluid(int container_size, float fluid_size, float diffusion, float viscosity, float dt) {
    this->container_size = container_size;
    this->fluid_size = fluid_size;  // raylib coordinate size of a single cell
    this->dt = dt;
    this->diffusion = diffusion;
    this->visc = viscosity;

    s = Field<float>();
    density = Field<float>();
    vx = Field<float>();
    vy = Field<float>();
    vz = Field<float>();
    vx0 = Field<float>();
    vy0 = Field<float>();
    vz0 = Field<float>();
    state = Field<CellType>();

    // not implemented
    // this->obstacles = std::vector<Obstacle>();
    // this->solid_frac = new float[array_size]();
}

Fluid::~Fluid(void) {
    for (const Obstacle& obstacle : obstacles) UnloadModel(obstacle.model);
}

float Fluid::get_density(v3 position) { return density[IXv(position)]; }
v3 Fluid::get_velocity(v3 position) {
    return {
        vx[IXv(position)],
        vy[IXv(position)],
        vz[IXv(position)],
    };
}

void Fluid::reset(void) {
    float base = 0.0f;

    s.fill(base);
    density.fill(base);
    vx.fill(base);
    vy.fill(base);
    vz.fill(base);
    vx0.fill(base);
    vy0.fill(base);
    vz0.fill(base);
}

void Fluid::add_density(v3 position, float amount) { this->density[IXv(position)] += amount; }

void Fluid::add_velocity(v3 position, v3 amount) {
    int index = IXv(position);
    vx[index] += amount.x;
    vy[index] += amount.y;
    vz[index] += amount.z;
}

void Fluid::advect(FieldType b, Field<float>& d, Field<float>& d0, Field<float>& velocX,
                   Field<float>& velocY, Field<float>& velocZ) {
    float i0, i1, j0, j1, k0, k1;

    float dtx = dt * (N - 2);
    float dty = dt * (N - 2);
    float dtz = dt * (N - 2);

    float s0, s1, t0, t1, u0, u1;
    float tmp1, tmp2, tmp3, x, y, z;

    float Nfloat = N;
    float ifloat, jfloat, kfloat;
    int i, j, k;

    for (k = 1, kfloat = 1; k < N - 1; k++, kfloat++) {
        for (j = 1, jfloat = 1; j < N - 1; j++, jfloat++) {
            for (i = 1, ifloat = 1; i < N - 1; i++, ifloat++) {
                tmp1 = dtx * velocX[IX(i, j, k)];
                tmp2 = dty * velocY[IX(i, j, k)];
                tmp3 = dtz * velocZ[IX(i, j, k)];
                x = ifloat - tmp1;
                y = jfloat - tmp2;
                z = kfloat - tmp3;

                if (x < 0.5f) x = 0.5f;
                if (x > Nfloat + 0.5f) x = Nfloat + 0.5f;
                i0 = floorf(x);
                i1 = i0 + 1.0f;
                if (y < 0.5f) y = 0.5f;
                if (y > Nfloat + 0.5f) y = Nfloat + 0.5f;
                j0 = floorf(y);
                j1 = j0 + 1.0f;
                if (z < 0.5f) z = 0.5f;
                if (z > Nfloat + 0.5f) z = Nfloat + 0.5f;
                k0 = floorf(z);
                k1 = k0 + 1.0f;

                s1 = x - i0;
                s0 = 1.0f - s1;
                t1 = y - j0;
                t0 = 1.0f - t1;
                u1 = z - k0;
                u0 = 1.0f - u1;

                int i0i = i0;
                int i1i = i1;
                int j0i = j0;
                int j1i = j1;
                int k0i = k0;
                int k1i = k1;

                d[IX(i, j, k)] =
                    s0 * (t0 * (u0 * d0[IX(i0i, j0i, k0i)] + u1 * d0[IX(i0i, j0i, k1i)]) +
                          (t1 * (u0 * d0[IX(i0i, j1i, k0i)] + u1 * d0[IX(i0i, j1i, k1i)]))) +
                    s1 * (t0 * (u0 * d0[IX(i1i, j0i, k0i)] + u1 * d0[IX(i1i, j0i, k1i)]) +
                          (t1 * (u0 * d0[IX(i1i, j1i, k0i)] + u1 * d0[IX(i1i, j1i, k1i)])));
            }
        }
    }
    set_boundaries(b, d);
}

void Fluid::diffuse(FieldType b, Field<float>& x, Field<float>& x0, float diff) {
    float a = dt * diff * pow(container_size - 2, 3);
    lin_solve(b, x, x0, a, 1 + 6 * a);
}

void Fluid::lin_solve(FieldType b, Field<float>& x, Field<float>& x0, float a, float c) {
    float cRecip = 1.0 / c;

    for (int k = 0; k < 4; k++) {
        for (int m = 1; m < N - 1; m++) {
            for (int j = 1; j < N - 1; j++) {
                for (int i = 1; i < N - 1; i++) {
                    x[IX(i, j, m)] =
                        (x0[IX(i, j, m)] +
                         a * (x[IX(i + 1, j, m)] + x[IX(i - 1, j, m)] + x[IX(i, j + 1, m)] +
                              x[IX(i, j - 1, m)] + x[IX(i, j, m + 1)] + x[IX(i, j, m - 1)])) *
                        cRecip;
                }
            }
        }
        set_boundaries(b, x);
    }
}

void Fluid::project(Field<float>& velocX, Field<float>& velocY, Field<float>& velocZ,
                    Field<float>& p, Field<float>& div) {
    // Calculate divergence
    for (int k = 1; k < N - 1; k++) {
        for (int j = 1; j < N - 1; j++) {
            for (int i = 1; i < N - 1; i++) {
                div[IX(i, j, k)] =
                    -0.5f *
                    (velocX[IX(i + 1, j, k)] - velocX[IX(i - 1, j, k)] + velocY[IX(i, j + 1, k)] -
                     velocY[IX(i, j - 1, k)] + velocZ[IX(i, j, k + 1)] - velocZ[IX(i, j, k - 1)]) /
                    N;
                p[IX(i, j, k)] = 0;
            }
        }
    }

    set_boundaries(FieldType::DENSITY, div);
    set_boundaries(FieldType::DENSITY, p);
    lin_solve(FieldType::DENSITY, p, div, 1, 6);

    // Adjust velocity based on pressure
    for (int k = 1; k < N - 1; k++) {
        for (int j = 1; j < N - 1; j++) {
            for (int i = 1; i < N - 1; i++) {
                velocX[IX(i, j, k)] -= 0.5f * (p[IX(i + 1, j, k)] - p[IX(i - 1, j, k)]) * N;
                velocY[IX(i, j, k)] -= 0.5f * (p[IX(i, j + 1, k)] - p[IX(i, j - 1, k)]) * N;
                velocZ[IX(i, j, k)] -= 0.5f * (p[IX(i, j, k + 1)] - p[IX(i, j, k - 1)]) * N;
            }
        }
    }

    set_boundaries(FieldType::VX, velocX);
    set_boundaries(FieldType::VY, velocY);
    set_boundaries(FieldType::VZ, velocZ);
}

void Fluid::set_boundaries(FieldType b, Field<float>& f) {
    for (int j = 1; j < N - 1; j++) {
        for (int i = 1; i < N - 1; i++) {
            f[IX(i, j, 0)] = b == FieldType::VZ ? -f[IX(i, j, 1)] : f[IX(i, j, 1)];
            f[IX(i, j, N - 1)] = b == FieldType::VZ ? -f[IX(i, j, N - 2)] : f[IX(i, j, N - 2)];
        }
    }

    for (int k = 1; k < N - 1; k++) {
        for (int i = 1; i < N - 1; i++) {
            f[IX(i, 0, k)] = b == FieldType::VY ? -f[IX(i, 1, k)] : f[IX(i, 1, k)];
            f[IX(i, N - 1, k)] = b == FieldType::VY ? -f[IX(i, N - 2, k)] : f[IX(i, N - 2, k)];
        }
    }

    for (int k = 1; k < N - 1; k++) {
        for (int j = 1; j < N - 1; j++) {
            f[IX(0, j, k)] = b == FieldType::VX ? -f[IX(1, j, k)] : f[IX(1, j, k)];
            f[IX(N - 1, j, k)] = b == FieldType::VX ? -f[IX(N - 2, j, k)] : f[IX(N - 2, j, k)];
        }
    }

    f[IX(0, 0, 0)] = 0.33f * (f[IX(1, 0, 0)] + f[IX(0, 1, 0)] + f[IX(0, 0, 1)]);
    f[IX(0, N - 1, 0)] = 0.33f * (f[IX(1, N - 1, 0)] + f[IX(0, N - 2, 0)] + f[IX(0, N - 1, 1)]);
    f[IX(0, 0, N - 1)] = 0.33f * (f[IX(1, 0, N - 1)] + f[IX(0, 1, N - 1)] + f[IX(0, 0, N)]);
    f[IX(0, N - 1, N - 1)] =
        0.33f * (f[IX(1, N - 1, N - 1)] + f[IX(0, N - 2, N - 1)] + f[IX(0, N - 1, N - 2)]);
    f[IX(N - 1, 0, 0)] = 0.33f * (f[IX(N - 2, 0, 0)] + f[IX(N - 1, 1, 0)] + f[IX(N - 1, 0, 1)]);
    f[IX(N - 1, N - 1, 0)] =
        0.33f * (f[IX(N - 2, N - 1, 0)] + f[IX(N - 1, N - 2, 0)] + f[IX(N - 1, N - 1, 1)]);
    f[IX(N - 1, 0, N - 1)] =
        0.33f * (f[IX(N - 2, 0, N - 1)] + f[IX(N - 1, 1, N - 1)] + f[IX(N - 1, 0, N - 2)]);
    f[IX(N - 1, N - 1, N - 1)] = 0.33f * (f[IX(N - 2, N - 1, N - 1)] + f[IX(N - 1, N - 2, N - 1)] +
                                          f[IX(N - 1, N - 1, N - 2)]);
}

void Fluid::step() {
    diffuse(FieldType::VX, vx0, vx, visc);
    diffuse(FieldType::VY, vy0, vy, visc);
    diffuse(FieldType::VZ, vz0, vz, visc);

    project(vx0, vy0, vz0, vx, vy);

    advect(FieldType::VX, vx, vx0, vx0, vy0, vz0);
    advect(FieldType::VY, vy, vy0, vx0, vy0, vz0);
    advect(FieldType::VZ, vz, vz0, vx0, vy0, vz0);

    project(vx, vy, vz, vx0, vy0);

    diffuse(FieldType::DENSITY, s, density, diffusion);
    advect(FieldType::DENSITY, density, s, vx, vy, vz);
}

void Fluid::add_obstacle(v3 position, v3 size, Model model) {
    Obstacle obstacle = {position, size, model};
    obstacles.push_back(obstacle);
    voxelize(obstacle);
}

void Fluid::voxelize(Obstacle obstacle) {
    for (int z = 0; z < N; z++) {
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                // ClassifyCell({x, y, z}, {1, 1, 1}, obstacle.position, obstacle.size);

                v3 min_cell = v3(x, y, z);
                v3 max_cell = v3(x + 1, y + 1, z + 1);

                v3 min_model = obstacle.position - obstacle.size / 2;
                v3 max_model = obstacle.position + obstacle.size / 2;

                if (!((min_cell.x <= max_model.x && max_cell.x >= min_model.x) &&
                      (min_cell.y <= max_model.y && max_cell.y >= min_model.y) &&
                      (min_cell.z <= max_model.z && max_cell.z >= min_model.z))) {
                    state[IXv(min_cell)] = CellType::FLUID;
                    continue;
                }

                if (min_cell.x >= min_model.x && max_cell.x <= max_model.x &&
                    min_cell.y >= min_model.y && max_cell.y <= max_model.y &&
                    min_cell.z >= min_model.z && max_cell.z <= max_model.z) {
                    state[IXv(min_cell)] = CellType::SOLID;
                    continue;
                }

                state[IXv(min_cell)] = CellType::CUT_CELL;
            }
        }
    }
}

CellType Fluid::get_state(v3 position) { return state[IXv(position)]; }
