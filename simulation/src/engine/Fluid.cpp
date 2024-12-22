#include "../../include/engine/Fluid.hpp"

#include <omp.h>
#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cstring>
#include <memory>
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
    state.fill(CellType::FLUID);

    boundary_cache = std::vector<float>();
    boundary_cells = std::vector<v3>();
    is_boundary_dirty = true;
}

Fluid::~Fluid(void) {}

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
#pragma omp parallel for collapse(2)
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
    // Calculate divergence
    for (int z = 1; z < N - 1; z++) {
        for (int y = 1; y < N - 1; y++) {
            for (int x = 1; x < N - 1; x++) {
                if (state[IX(x, y, z)] == CellType::SOLID) {
                    div[IX(x, y, z)] = 0;  // No divergence in solid cells
                    p[IX(x, y, z)] = 0;    // Pressure is also zero
                } else if (state[IX(x, y, z)] == CellType::CUT_CELL) {
                    float fraction = get_fractional_volume(v3(x, y, z));
                    div[IX(x, y, z)] = -0.5f * fraction *
                                       (velocX[IX(x + 1, y, z)] - velocX[IX(x - 1, y, z)] +
                                        velocY[IX(x, y + 1, z)] - velocY[IX(x, y - 1, z)] +
                                        velocZ[IX(x, y, z + 1)] - velocZ[IX(x, y, z - 1)]) /
                                       N;
                    p[IX(x, y, z)] = 0;
                } else {  // FLUID cells
                    div[IX(x, y, z)] = -0.5f *
                                       (velocX[IX(x + 1, y, z)] - velocX[IX(x - 1, y, z)] +
                                        velocY[IX(x, y + 1, z)] - velocY[IX(x, y - 1, z)] +
                                        velocZ[IX(x, y, z + 1)] - velocZ[IX(x, y, z - 1)]) /
                                       N;
                    p[IX(x, y, z)] = 0;
                }
            }
        }
    }

    // Apply boundary conditions for divergence and pressure
    set_boundaries(FieldType::DENSITY, div);
    set_boundaries(FieldType::DENSITY, p);

    // Solve for pressure
    lin_solve(FieldType::DENSITY, p, div, 1, 6);

    // Adjust velocity based on the pressure gradient
    for (int k = 1; k < N - 1; k++) {
        for (int j = 1; j < N - 1; j++) {
            for (int i = 1; i < N - 1; i++) {
                if (state[IX(i, j, k)] == CellType::SOLID) {
                    velocX[IX(i, j, k)] = 0;
                    velocY[IX(i, j, k)] = 0;
                    velocZ[IX(i, j, k)] = 0;
                } else if (state[IX(i, j, k)] == CellType::CUT_CELL) {
                    float fraction = get_fractional_volume(v3(i, j, k));
                    velocX[IX(i, j, k)] -=
                        0.5f * fraction * (p[IX(i + 1, j, k)] - p[IX(i - 1, j, k)]) * N;
                    velocY[IX(i, j, k)] -=
                        0.5f * fraction * (p[IX(i, j + 1, k)] - p[IX(i, j - 1, k)]) * N;
                    velocZ[IX(i, j, k)] -=
                        0.5f * fraction * (p[IX(i, j, k + 1)] - p[IX(i, j, k - 1)]) * N;
                } else {  // FLUID cells
                    velocX[IX(i, j, k)] -= 0.5f * (p[IX(i + 1, j, k)] - p[IX(i - 1, j, k)]) * N;
                    velocY[IX(i, j, k)] -= 0.5f * (p[IX(i, j + 1, k)] - p[IX(i, j - 1, k)]) * N;
                    velocZ[IX(i, j, k)] -= 0.5f * (p[IX(i, j, k + 1)] - p[IX(i, j, k - 1)]) * N;
                }
            }
        }
    }

    // Apply boundary conditions for velocity
    set_boundaries(FieldType::VX, velocX);
    set_boundaries(FieldType::VY, velocY);
    set_boundaries(FieldType::VZ, velocZ);
}

void Fluid::set_boundaries(FieldType b, Field<float>& f) {
    // Handle each face of the bounding box
    for (int y = 1; y < N - 1; y++) {
        for (int x = 1; x < N - 1; x++) {
            int index0 = IX(x, y, 0);
            int indexN = IX(x, y, N - 1);

            // Handle bottom (z=0) and top (z=N-1) boundaries
            if (state[index0] == CellType::SOLID) {
                f[index0] = 0.0f;  // No velocity through solid
            } else if (b == FieldType::VZ) {
                f[index0] = -f[IX(x, y, 1)];
            } else {
                f[index0] = f[IX(x, y, 1)];
            }

            if (state[indexN] == CellType::SOLID) {
                f[indexN] = 0.0f;
            } else if (b == FieldType::VZ) {
                f[indexN] = -f[IX(x, y, N - 2)];
            } else {
                f[indexN] = f[IX(x, y, N - 2)];
            }
        }
    }

    for (int z = 1; z < N - 1; z++) {
        for (int x = 1; x < N - 1; x++) {
            int index0 = IX(x, 0, z);
            int indexN = IX(x, N - 1, z);

            // Handle front (y=0) and back (y=N-1) boundaries
            if (state[index0] == CellType::SOLID) {
                f[index0] = 0.0f;
            } else if (b == FieldType::VY) {
                f[index0] = -f[IX(x, 1, z)];
            } else {
                f[index0] = f[IX(x, 1, z)];
            }

            if (state[indexN] == CellType::SOLID) {
                f[indexN] = 0.0f;
            } else if (b == FieldType::VY) {
                f[indexN] = -f[IX(x, N - 2, z)];
            } else {
                f[indexN] = f[IX(x, N - 2, z)];
            }
        }
    }

    for (int z = 1; z < N - 1; z++) {
        for (int y = 1; y < N - 1; y++) {
            int index0 = IX(0, y, z);
            int indexN = IX(N - 1, y, z);

            if (state[index0] == CellType::SOLID) {
                f[index0] = 0.0f;
            } else if (b == FieldType::VX) {
                f[index0] = -f[IX(1, y, z)];
            } else {
                f[index0] = f[IX(1, y, z)];
            }

            if (state[indexN] == CellType::SOLID) {
                f[indexN] = 0.0f;
            } else if (b == FieldType::VX) {
                f[indexN] = -f[IX(N - 2, y, z)];
            } else {
                f[indexN] = f[IX(N - 2, y, z)];
            }
        }
    }

    // Handle corners (ensure no fluid leakage)
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

void Fluid::add_obstacle(v3 position, float size, Model model) {
    Obstacle obstacle(position, size, model);
    obstacles.push_back(obstacle);
    voxelize(obstacle);
    is_boundary_dirty = true;
}

void Fluid::voxelize(Obstacle obstacle) {
    for (int z = 0; z < N; z++) {
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                // Define the voxel's position and size
                v3 cell_position(x + 0.5f, y + 0.5f, z + 0.5f);  // Center of the voxel
                v3 cell_size(1.0f, 1.0f, 1.0f);                  // Voxel dimensions

                // Create the voxel geometry and wrap it in a shared_ptr
                auto cell_box_ptr =
                    std::make_shared<fcl::Boxf>(cell_size.x, cell_size.y, cell_size.z);

                // Create transforms for the voxel and obstacle
                fcl::Transform3f cell_transform(
                    fcl::Translation3f(cell_position.x, cell_position.y, cell_position.z));
                fcl::Transform3f obstacle_transform(fcl::Translation3f(
                    obstacle.position.x, obstacle.position.y, obstacle.position.z));

                // Create FCL collision objects
                fcl::CollisionObject<float> voxel_obj(cell_box_ptr, cell_transform);
                // fcl::CollisionObject<float> obstacle_obj(obstacle.geom, obstacle_transform);
                fcl::CollisionObject<float> obstacle_obj(
                    std::shared_ptr<fcl::BVHModel<fcl::OBBf>>(obstacle.geom.get()),
                    obstacle_transform);

                // Collision request and result
                fcl::CollisionRequest<float> collision_request;
                fcl::CollisionResult<float> collision_result;

                // Perform collision query
                fcl::collide(&voxel_obj, &obstacle_obj, collision_request, collision_result);

                // Classification logic
                if (collision_result.isCollision()) {
                    // Check if the voxel is fully contained in the obstacle
                    fcl::AABBf cell_aabb(cell_position - (cell_size / 2),
                                         cell_position + (cell_size / 2));

                    if (cell_aabb.center().cwiseAbs().array().all() <=
                        obstacle.geom->aabb_center.cwiseAbs().array().all()) {
                        state[IX(x, y, z)] = CellType::SOLID;
                    } else {
                        state[IX(x, y, z)] = CellType::CUT_CELL;
                    }
                } else {
                    state[IX(x, y, z)] = CellType::FLUID;
                }
            }
        }
    }
}

float Fluid::get_fractional_volume(v3 position) {
    BoundingBox cell = {position, position + v3(1)};
    float total_intersection_volume = 0.0f;

    // Loop through all obstacles
    for (Obstacle& obstacle : obstacles) {
        BoundingBox obs = {
            obstacle.position - v3(obstacle.size / 2.0f),
            obstacle.position + v3(obstacle.size / 2.0f),
        };

        // Calculate intersection bounds
        BoundingBox intersect = {
            v3(std::max(cell.min.x, obs.min.x), std::max(cell.min.y, obs.min.y),
               std::max(cell.min.z, obs.min.z)),
            v3(std::min(cell.max.x, obs.max.x), std::min(cell.max.y, obs.max.y),
               std::min(cell.max.z, obs.max.z))};

        // Calculate intersection dimensions
        float dx = std::max(0.0f, intersect.max.x - intersect.min.x);
        float dy = std::max(0.0f, intersect.max.y - intersect.min.y);
        float dz = std::max(0.0f, intersect.max.z - intersect.min.z);

        // Add to total intersection volume
        total_intersection_volume += dx * dy * dz;
    }

    // Fractional volume is the portion of the cell not occupied by obstacles
    float cell_volume = 1.0f;  // Each cell is a unit cube
    return 1.0f - (total_intersection_volume / cell_volume);
}

CellType Fluid::get_state(v3 position) { return state[IXv(position)]; }
