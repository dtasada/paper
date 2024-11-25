#include "../../include/engine/Fluid.hpp"

#include <raylib.h>
#include <raymath.h>

#include <cstring>

#include "../../include/engine/Engine.hpp"

Fluid::Fluid(int container_size, float fluid_size, float diffusion, float viscosity, float dt) {
    this->container_size = container_size;
    this->fluid_size = fluid_size;  // raylib coordinate size of a single cell
    this->dt = dt;
    this->diffusion = diffusion;
    this->visc = viscosity;

    int array_size = pow(container_size, 3);
    this->s = new float[array_size]();
    this->solid = new bool[array_size]();
    this->density = new float[array_size]();
    this->vx = new float[array_size]();
    this->vy = new float[array_size]();
    this->vz = new float[array_size]();
    this->vx0 = new float[array_size]();
    this->vy0 = new float[array_size]();
    this->vz0 = new float[array_size]();
}

Fluid::~Fluid() {
    delete[] this->s;
    delete[] this->solid;
    delete[] this->density;
    delete[] this->vx;
    delete[] this->vy;
    delete[] this->vz;
    delete[] this->vx0;
    delete[] this->vy0;
    delete[] this->vz0;
}

float Fluid::get_density(v3 position) { return density[IXv(position)]; }
v3 Fluid::get_velocity(v3 position) {
    return {
        vx[IXv(position)],
        vy[IXv(position)],
        vz[IXv(position)],
    };
}
bool Fluid::is_solid(v3 position) { return solid[IXv(position)]; }
void Fluid::set_solid(v3 position, bool set) { solid[IXv(position)] = set; }

void Fluid::reset() {
    int array_size = pow(container_size, 3);
    float base = 0.0f;
    std::fill(s, s + array_size, base);
    std::fill(density, density + array_size, base);
    std::fill(vx, vx + array_size, base);
    std::fill(vy, vy + array_size, base);
    std::fill(vz, vz + array_size, base);
    std::fill(vx0, vx0 + array_size, base);
    std::fill(vy0, vy0 + array_size, base);
    std::fill(vz0, vz0 + array_size, base);
}

void Fluid::add_gravity() {
    int N = container_size;
    float gravity = -9.81f;

    for (int z = 1; z < N - 1; z++) {
        for (int y = 1; y < N - 1; y++) {
            for (int x = 1; x < N - 1; x++) {
                add_velocity(v3(x, y, z), v3(0.0f, gravity, 0.0f));
            }
        }
    }

    set_boundaries(2, vy);
}

void Fluid::add_density(v3 position, float amount) { this->density[IXv(position)] += amount; }

void Fluid::add_velocity(v3 position, v3 amount) {
    int index = IXv(position);
    vx[index] += amount.x;
    vy[index] += amount.y;
    vz[index] += amount.z;
}

void Fluid::advect(int b, float *d, float *d0, float *velocX, float *velocY, float *velocZ) {
    float N = container_size;

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
                if (solid[IX(i, j, k)]) {
                    if (b >= 1 && b <= 3) {  // For velocity components
                        d[IX(i, j, k)] = 0;
                    }
                    continue;
                }

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

void Fluid::diffuse(int b, float *x, float *x0, float diff) {
    float a = dt * diff * pow(container_size - 2, 3);
    lin_solve(b, x, x0, a, 1 + 6 * a);
}

void Fluid::lin_solve(int b, float *x, float *x0, float a, float c) {
    float cRecip = 1.0 / c;
    int N = container_size;

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

void Fluid::project(float *velocX, float *velocY, float *velocZ, float *p, float *div) {
    int N = container_size;

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

    set_boundaries(0, div);
    set_boundaries(0, p);
    lin_solve(0, p, div, 1, 6);

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

    set_boundaries(1, velocX);
    set_boundaries(2, velocY);
    set_boundaries(3, velocZ);
}

void Fluid::set_boundaries(int b, float *f) {
    int N = container_size;

    // Handle solid boundaries first
    for (int z = 1; z < N - 1; z++) {
        for (int y = 1; y < N - 1; y++) {
            for (int x = 1; x < N - 1; x++) {
                if (solid[IX(x, y, z)]) {
                    // For velocity components, enforce no-slip condition
                    if (b == 1) f[IX(x, y, z)] = 0;  // x velocity
                    if (b == 2) f[IX(x, y, z)] = 0;  // y velocity
                    if (b == 3) f[IX(x, y, z)] = 0;  // z velocity

                    // For density and pressure, use average of neighboring non-solid cells
                    if (b == 0) {
                        float sum = 0;
                        int count = 0;

                        if (!solid[IX(x - 1, y, z)]) {
                            sum += f[IX(x - 1, y, z)];
                            count++;
                        }
                        if (!solid[IX(x + 1, y, z)]) {
                            sum += f[IX(x + 1, y, z)];
                            count++;
                        }
                        if (!solid[IX(x, y - 1, z)]) {
                            sum += f[IX(x, y - 1, z)];
                            count++;
                        }
                        if (!solid[IX(x, y + 1, z)]) {
                            sum += f[IX(x, y + 1, z)];
                            count++;
                        }
                        if (!solid[IX(x, y, z - 1)]) {
                            sum += f[IX(x, y, z - 1)];
                            count++;
                        }
                        if (!solid[IX(x, y, z + 1)]) {
                            sum += f[IX(x, y, z + 1)];
                            count++;
                        }

                        f[IX(x, y, z)] = count > 0 ? sum / count : f[IX(x, y, z)];
                    }
                }
            }
        }
    }

    for (int j = 1; j < N - 1; j++) {
        for (int i = 1; i < N - 1; i++) {
            f[IX(i, j, 0)] = b == 3 ? -f[IX(i, j, 1)] : f[IX(i, j, 1)];
            f[IX(i, j, N - 1)] = b == 3 ? -f[IX(i, j, N - 2)] : f[IX(i, j, N - 2)];
        }
    }

    for (int k = 1; k < N - 1; k++) {
        for (int i = 1; i < N - 1; i++) {
            f[IX(i, 0, k)] = b == 2 ? -f[IX(i, 1, k)] : f[IX(i, 1, k)];
            f[IX(i, N - 1, k)] = b == 2 ? -f[IX(i, N - 2, k)] : f[IX(i, N - 2, k)];
        }
    }

    for (int k = 1; k < N - 1; k++) {
        for (int j = 1; j < N - 1; j++) {
            f[IX(0, j, k)] = b == 1 ? -f[IX(1, j, k)] : f[IX(1, j, k)];
            f[IX(N - 1, j, k)] = b == 1 ? -f[IX(N - 2, j, k)] : f[IX(N - 2, j, k)];
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
    // add_gravity();
    diffuse(1, vx0, vx, visc);
    diffuse(2, vy0, vy, visc);
    diffuse(3, vz0, vz, visc);

    project(vx0, vy0, vz0, vx, vy);

    advect(1, vx, vx0, vx0, vy0, vz0);
    advect(2, vy, vy0, vx0, vy0, vz0);
    advect(3, vz, vz0, vx0, vy0, vz0);

    project(vx, vy, vz, vx0, vy0);

    diffuse(0, s, density, diffusion);
    advect(0, density, s, vx, vy, vz);
}

void Fluid::add_cube(v3 pos, float size) {
    for (int z = pos.z; z < pos.z + size && z < container_size; z++) {
        for (int y = pos.y; y < pos.y + size && y < container_size; y++) {
            for (int x = pos.x; x < pos.y + size && x < container_size; x++) {
                solid[IX(x, y, z)] = true;
            }
        }
    }
}
