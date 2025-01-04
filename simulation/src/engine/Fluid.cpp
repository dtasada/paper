#include "../../include/engine/Fluid.hpp"

#include <fcl/common/types.h>

#include "../../include/engine/engine.hpp"

Fluid::Fluid(int container_size, float scaling, float diffusion, float viscosity, float dt) {
    this->container_size = container_size;
    this->scaling = scaling;  // raylib world size of a single cell
    this->dt = dt;
    this->diffusion = diffusion;
    this->visc = viscosity;

    s = Field<float>(N3);
    density = Field<float>(N3);
    vx = Field<float>(N3);
    vy = Field<float>(N3);
    vz = Field<float>(N3);
    vx0 = Field<float>(N3);
    vy0 = Field<float>(N3);
    vz0 = Field<float>(N3);
    state = Field<CellType>(N3, CellType::UNDEFINED);
    volume = Field<float>(N3);

    obstacles = std::vector<std::unique_ptr<Obstacle>>();

    should_voxelize = false;
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
    s = Field<float>(N3);
    density = Field<float>(N3);
    vx = Field<float>(N3);
    vy = Field<float>(N3);
    vz = Field<float>(N3);
    vx0 = Field<float>(N3);
    vy0 = Field<float>(N3);
    vz0 = Field<float>(N3);
    state = Field<CellType>(N3, CellType::UNDEFINED);

    voxelize_all();
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
    float dtx = dt * (N - 2);
    float dty = dt * (N - 2);
    float dtz = dt * (N - 2);

#pragma omp parallel for collapse(3)
    for (int k = 1; k < N - 1; k++) {
        for (int j = 1; j < N - 1; j++) {
            for (int i = 1; i < N - 1; i++) {
                float tmp1 = dtx * velocX[IX(i, j, k)];
                float tmp2 = dty * velocY[IX(i, j, k)];
                float tmp3 = dtz * velocZ[IX(i, j, k)];
                float x = i - tmp1;
                float y = j - tmp2;
                float z = k - tmp3;

                if (x < 0.5f) x = 0.5f;
                if (x > N + 0.5f) x = N + 0.5f;
                float i0 = floorf(x);
                float i1 = i0 + 1.0f;
                if (y < 0.5f) y = 0.5f;
                if (y > N + 0.5f) y = N + 0.5f;
                float j0 = floorf(y);
                float j1 = j0 + 1.0f;
                if (z < 0.5f) z = 0.5f;
                if (z > N + 0.5f) z = N + 0.5f;
                float k0 = floorf(z);
                float k1 = k0 + 1.0f;

                float s1 = x - i0;
                float s0 = 1.0f - s1;
                float t1 = y - j0;
                float t0 = 1.0f - t1;
                float u1 = z - k0;
                float u0 = 1.0f - u1;

                d[IX(i, j, k)] =
                    s0 * (t0 * (u0 * d0[IX(i0, j0, k0)] + u1 * d0[IX(i0, j0, k1)]) +
                          (t1 * (u0 * d0[IX(i0, j1, k0)] + u1 * d0[IX(i0, j1, k1)]))) +
                    s1 * (t0 * (u0 * d0[IX(i1, j0, k0)] + u1 * d0[IX(i1, j0, k1)]) +
                          (t1 * (u0 * d0[IX(i1, j1, k0)] + u1 * d0[IX(i1, j1, k1)])));
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
#pragma omp parallel for collapse(2)
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
                    float fraction = volume[IX(x, y, z)];
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
    for (int z = 1; z < N - 1; z++) {
        for (int y = 1; y < N - 1; y++) {
            for (int x = 1; x < N - 1; x++) {
                if (state[IX(x, y, z)] == CellType::SOLID) {
                    velocX[IX(x, y, z)] = 0;
                    velocY[IX(x, y, z)] = 0;
                    velocZ[IX(x, y, z)] = 0;
                } else if (state[IX(x, y, z)] == CellType::CUT_CELL) {
                    float fraction = volume[IX(x, y, z)];
                    velocX[IX(x, y, z)] -=
                        0.5f * fraction * (p[IX(x + 1, y, z)] - p[IX(x - 1, y, z)]) * N;
                    velocY[IX(x, y, z)] -=
                        0.5f * fraction * (p[IX(x, y + 1, z)] - p[IX(x, y - 1, z)]) * N;
                    velocZ[IX(x, y, z)] -=
                        0.5f * fraction * (p[IX(x, y, z + 1)] - p[IX(x, y, z - 1)]) * N;
                } else {  // FLUID cells
                    velocX[IX(x, y, z)] -= 0.5f * (p[IX(x + 1, y, z)] - p[IX(x - 1, y, z)]) * N;
                    velocY[IX(x, y, z)] -= 0.5f * (p[IX(x, y + 1, z)] - p[IX(x, y - 1, z)]) * N;
                    velocZ[IX(x, y, z)] -= 0.5f * (p[IX(x, y, z + 1)] - p[IX(x, y, z - 1)]) * N;
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
#pragma omp parallel for collapse(2)
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

#pragma omp parallel for collapse(2)
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

#pragma omp parallel for collapse(2)
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

void Fluid::add_obstacle(std::unique_ptr<Obstacle> obstacle) {
    obstacles.push_back(std::move(obstacle));
    voxelize_all();
}

void Fluid::voxelize(Obstacle& obstacle) {
    // Prepare the obstacle collision object
    obstacle.geom->computeLocalAABB();
    fcl::CollisionObjectf obstacle_obj(obstacle.geom,
                                       fcl::Transform3f(fcl::Translation3f(obstacle.position)));

#pragma omp parallel for collapse(3)
    for (int z = 0; z < N; z++) {
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                // Define the voxel's position and size
                v3 cell_position(x, y, z);
                v3 cell_size(1.0f, 1.0f, 1.0f);

                // Create a voxel collision object
                auto cell_geometry =
                    std::make_shared<fcl::Boxf>(cell_size.x, cell_size.y, cell_size.z);
                fcl::CollisionObjectf cell_obj(
                    cell_geometry, fcl::Transform3f(fcl::Translation3f(cell_position)));

                // Perform collision query
                fcl::CollisionRequestf request;
                fcl::CollisionResultf result;
                fcl::collide(&cell_obj, &obstacle_obj, request, result);

                // Debug collision results
                std::cout << "Voxel (" << x << ", " << y << ", " << z
                          << ") Collision: " << result.isCollision()
                          << ", Contacts: " << result.numContacts() << std::endl;

                // Classification logic
                if (result.isCollision()) {
                    // Simplify: Assume SOLID if collision exists (for debugging)
                    state[IX(x, y, z)] = CellType::SOLID;

                    // Uncomment below for detailed classification
                    /*
                    // Define the voxel's AABB
                    fcl::AABBf cell_aabb(cell_position, cell_position + cell_size);

                    // Check if all contact points are inside the voxel's AABB
                    bool fully_inside = true;
                    for (int i = 0; i < result.numContacts(); i++) {
                        const auto& contact = result.getContact(i);
                        std::cout << "Contact Point: " << contact.pos.transpose() << std::endl;
                        if (!cell_aabb.contains(contact.pos)) {
                            fully_inside = false;
                            break;
                        }
                    }

                    if (fully_inside) {
                        state[IX(x, y, z)] = CellType::SOLID;
                    } else {
                        state[IX(x, y, z)] = CellType::CUT_CELL;
                    }
                    */
                } else {
                    state[IX(x, y, z)] = CellType::FLUID;
                }
            }
        }
    }

    should_voxelize = false;
}

void Fluid::voxelize_all() {
    bool no_obstacles = true;
    for (auto& obstacle : obstacles) {
        if (obstacle->enabled) {
            no_obstacles = false;
            break;
        }
    }

    if (no_obstacles) {
        should_voxelize = false;
        state = Field<CellType>(N3, CellType::FLUID);
    } else {
        state = Field<CellType>(N3, CellType::UNDEFINED);
#pragma omp parallel for
        for (auto& obstacle : obstacles)
            if (obstacle->enabled) voxelize(*obstacle);
    }
}

float Fluid::get_volume(v3 cell_position) {
    v3 cell_size(1.0f, 1.0f, 1.0f);
    fcl::AABBf cell_aabb(cell_position, cell_position + cell_size);

    for (auto& obstacle : obstacles) {
        fcl::AABBf obstacle_aabb(
            obstacle->geom->aabb_local.min_ + (fcl::Vector3f)obstacle->position,
            obstacle->geom->aabb_local.max_ + (fcl::Vector3f)obstacle->position);

        fcl::AABBf intersection_aabb;
        intersection_aabb.min_ = cell_aabb.min_.cwiseMax(obstacle_aabb.min_);
        intersection_aabb.max_ = cell_aabb.max_.cwiseMin(obstacle_aabb.max_);

        fcl::Vector3f intersection_size = intersection_aabb.max_ - intersection_aabb.min_;
        if (intersection_size[0] <= 0 || intersection_size[1] <= 0 || intersection_size[2] <= 0) {
            return 0.0f;
        }

        float intersection_volume =
            intersection_size[0] * intersection_size[1] * intersection_size[2];

        float cell_volume = cell_size.x * cell_size.y * cell_size.z;
        return intersection_volume / cell_volume;
    }

    return 1.0f;
}

// float Fluid::get_volume(v3 cell_position) { return volume[IXv(cell_position)]; }

CellType Fluid::get_state(v3 position) { return state[IXv(position)]; }
