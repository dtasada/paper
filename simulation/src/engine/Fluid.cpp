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

    boundary_cache = std::vector<float>();
    boundary_cells = std::vector<v3>();
    is_boundary_dirty = true;
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

void Fluid::lin_solve(FieldType b, Field<float>& f, Field<float>& f0, float a, float c) {
    float cRecip = 1.0f / c;

    for (int i = 0; i < 4; i++) {
        for (int z = 1; z < N - 1; z++) {
            for (int y = 1; y < N - 1; y++) {
                for (int x = 1; x < N - 1; x++) {
                    f[IX(x, y, z)] =
                        (f0[IX(x, y, z)] +
                         a * (f[IX(x + 1, y, z)] + f[IX(x - 1, y, z)] + f[IX(x, y + 1, z)] +
                              f[IX(x, y - 1, z)] + f[IX(x, y, z + 1)] + f[IX(x, y, z - 1)])) *
                        cRecip;
                }
            }
        }
        set_boundaries(b, f);
    }
}

void Fluid::project(Field<float>& velocX, Field<float>& velocY, Field<float>& velocZ,
                    Field<float>& p, Field<float>& div) {
    for (int z = 1; z < N - 1; z++) {
        for (int y = 1; y < N - 1; y++) {
            for (int x = 1; x < N - 1; x++) {
                int position = IX(x, y, z);
                if (state[position] == CellType::FLUID) {
                    div[position] = -0.5f *
                                    (velocX[IX(x + 1, y, z)] - velocX[IX(x - 1, y, z)] +
                                     velocY[IX(x, y + 1, z)] - velocY[IX(x, y - 1, z)] +
                                     velocZ[IX(x, y, z + 1)] - velocZ[IX(x, y, z - 1)]) /
                                    N;
                    p[position] = 0;
                } else if (state[position] == CellType::CUT_CELL) {
                    // Scale divergence and pressure by fractional volume
                    float fraction = get_fractional_volume(v3(x, y, z));  // Implement this
                    div[position] *= fraction;
                    p[position] *= fraction;
                }
            }
        }
    }

    set_boundaries(FieldType::DENSITY, div);
    set_boundaries(FieldType::DENSITY, p);
    lin_solve(FieldType::DENSITY, p, div, 1, 6);

    // Adjust velocity based on corrected pressure
    for (int z = 1; z < N - 1; z++) {
        for (int y = 1; y < N - 1; y++) {
            for (int x = 1; x < N - 1; x++) {
                int index = IX(x, y, z);
                if (state[index] == CellType::FLUID || state[index] == CellType::CUT_CELL) {
                    velocX[index] -= 0.5f * (p[IX(x + 1, y, z)] - p[IX(x - 1, y, z)]) * N;
                    velocY[index] -= 0.5f * (p[IX(x, y + 1, z)] - p[IX(x, y - 1, z)]) * N;
                    velocZ[index] -= 0.5f * (p[IX(x, y, z + 1)] - p[IX(x, y, z - 1)]) * N;
                }
            }
        }
    }

    set_boundaries(FieldType::VX, velocX);
    set_boundaries(FieldType::VY, velocY);
    set_boundaries(FieldType::VZ, velocZ);
}

void Fluid::set_boundaries(FieldType b, Field<float>& f) {
    for (int y = 1; y < N - 1; y++) {
        for (int x = 1; x < N - 1; x++) {
            f[IX(x, y, 0)] = b == FieldType::VZ ? -f[IX(x, y, 1)] : f[IX(x, y, 1)];
            f[IX(x, y, N - 1)] = b == FieldType::VZ ? -f[IX(x, y, N - 2)] : f[IX(x, y, N - 2)];
        }
    }

    for (int z = 1; z < N - 1; z++) {
        for (int x = 1; x < N - 1; x++) {
            f[IX(x, 0, z)] = b == FieldType::VY ? -f[IX(x, 1, z)] : f[IX(x, 1, z)];
            f[IX(x, N - 1, z)] = b == FieldType::VY ? -f[IX(x, N - 2, z)] : f[IX(x, N - 2, z)];
        }
    }

    for (int z = 1; z < N - 1; z++) {
        for (int y = 1; y < N - 1; y++) {
            f[IX(0, y, z)] = b == FieldType::VX ? -f[IX(1, y, z)] : f[IX(1, y, z)];
            f[IX(N - 1, y, z)] = b == FieldType::VX ? -f[IX(N - 2, y, z)] : f[IX(N - 2, y, z)];
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

/* void Fluid::set_boundaries(FieldType b, Field<float>& f) {
    // If boundary cache is dirty, update the cache
    if (is_boundary_dirty) {
        // Iterate over all boundary cells to update the cache
        for (v3 position : boundary_cells) {
            int index = IXv(position);
            CellType cellType = state[index];

            if (cellType == CellType::SOLID) {
                boundary_cache[index] = 0.0f;  // For solid cells, set value to 0 (no velocity)
            } else if (cellType == CellType::CUT_CELL) {
                // For cut cells, handle them based on fluid behavior, not just negating
                int x = position.x;
                int y = position.y;
                int z = position.z;

                if (b == FieldType::VX) {
                    // Do not reflect in CUT_CELL cells, just pass-through behavior (depending on
                    // the simulation)
                    boundary_cache[index] = f[IX(x + 1, y, z)];  // Use adjacent fluid velocity
                } else if (b == FieldType::VY) {
                    boundary_cache[index] = f[IX(x, y + 1, z)];  // Use adjacent fluid velocity
                } else if (b == FieldType::VZ) {
                    boundary_cache[index] = f[IX(x, y, z + 1)];  // Use adjacent fluid velocity
                }
            }
        }
        // After updating, set the dirty flag to false
        is_boundary_dirty = false;
    }

    // Apply the cached values to the actual field
    for (v3 position : boundary_cells) {
        f[IXv(position)] = boundary_cache[IXv(position)];
    }

    // Additional handling for boundary cells outside the cache (e.g., corners)
    for (int z = 1; z < N - 1; z++) {
        for (int y = 1; y < N - 1; y++) {
            for (int x = 1; x < N - 1; x++) {
                int index = IX(z, y, x);

                // Handling boundaries for velocity fields (VX, VY, VZ)
                if (b == FieldType::VX) {
                    if (z == 0) {
                        // Reflect velocity at left boundary
                        f[IX(0, y, x)] = f[IX(1, y, x)];
                    } else if (z == N - 1) {
                        // Reflect velocity at right boundary
                        f[IX(N - 1, y, x)] = f[IX(N - 2, y, x)];
                    }
                }

                if (b == FieldType::VY) {
                    if (y == 0) {
                        // Reflect velocity at bottom boundary
                        f[IX(z, 0, x)] = f[IX(z, 1, x)];
                    } else if (y == N - 1) {
                        // Reflect velocity at top boundary
                        f[IX(z, N - 1, x)] = f[IX(z, N - 2, x)];
                    }
                }

                if (b == FieldType::VZ) {
                    if (x == 0) {
                        // Reflect velocity at front boundary
                        f[IX(z, y, 0)] = f[IX(z, y, 1)];
                    } else if (x == N - 1) {
                        // Reflect velocity at back boundary
                        f[IX(z, y, N - 1)] = f[IX(z, y, N - 2)];
                    }
                }
            }
        }
    }

    // Corner Handling: Average values at the corners (adjusted to avoid instability)
    f[IX(0, 0, 0)] = 0.33f * (f[IX(1, 0, 0)] + f[IX(0, 1, 0)] + f[IX(0, 0, 1)]);
    f[IX(0, N - 1, 0)] = 0.33f * (f[IX(1, N - 1, 0)] + f[IX(0, N - 2, 0)] + f[IX(0, N - 1, 1)]);
    f[IX(0, 0, N - 1)] = 0.33f * (f[IX(1, 0, N - 1)] + f[IX(0, 1, N - 1)] + f[IX(0, 0, N - 2)]);
    f[IX(0, N - 1, N - 1)] =
        0.33f * (f[IX(1, N - 1, N - 1)] + f[IX(0, N - 2, N - 1)] + f[IX(0, N - 1, N - 2)]);
    f[IX(N - 1, 0, 0)] = 0.33f * (f[IX(N - 2, 0, 0)] + f[IX(N - 1, 1, 0)] + f[IX(N - 1, 0, 1)]);
    f[IX(N - 1, N - 1, 0)] =
        0.33f * (f[IX(N - 2, N - 1, 0)] + f[IX(N - 1, N - 2, 0)] + f[IX(N - 1, N - 1, 1)]);
    f[IX(N - 1, 0, N - 1)] =
        0.33f * (f[IX(N - 2, 0, N - 1)] + f[IX(N - 1, 1, N - 1)] + f[IX(N - 1, 0, N - 2)]);
    f[IX(N - 1, N - 1, N - 1)] = 0.33f * (f[IX(N - 2, N - 1, N - 1)] + f[IX(N - 1, N - 2, N - 1)] +
                                          f[IX(N - 1, N - 1, N - 2)]);
} */

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

void Fluid::add_obstacle(v3 position, float size, Model model) {
    Obstacle obstacle = {position, size, model};
    obstacles.push_back(obstacle);
    voxelize(obstacle);
    is_boundary_dirty = true;
}

void Fluid::voxelize(Obstacle obstacle) {
    for (int z = 0; z < N; z++) {
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                BoundingBox cell = {v3(x, y, z), v3(x, y, z) + v3(1)};
                BoundingBox model = {obstacle.position - obstacle.size / 2,
                                     obstacle.position + obstacle.size / 2};

                if (!((cell.min.x <= model.max.x && cell.max.x >= model.min.x) &&
                      (cell.min.y <= model.max.y && cell.max.y >= model.min.y) &&
                      (cell.min.z <= model.max.z && cell.max.z >= model.min.z))) {
                    state[IXv(cell.min)] = CellType::FLUID;
                    continue;
                }

                if (cell.min.x >= model.min.x && cell.max.x <= model.max.x &&
                    cell.min.y >= model.min.y && cell.max.y <= model.max.y &&
                    cell.min.z >= model.min.z && cell.max.z <= model.max.z) {
                    state[IXv(cell.min)] = CellType::SOLID;
                    continue;
                }

                state[IXv(cell.min)] = CellType::CUT_CELL;
            }
        }
    }
}

CellType Fluid::get_state(v3 position) { return state[IXv(position)]; }

float Fluid::get_fractional_volume(v3 position) {
    // Compute the fractional volume of a CUT_CELL
    // Based on voxelized geometry or external input
    return 0.5f;  // Placeholder
}
