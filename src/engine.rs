#![feature(try_blocks)]
use std::error;
use std::ptr::{write_volatile, read_volatile};

use raylib::color::Color;
use raylib::prelude::*;

/// # paper-engine
/// Functions & equations adapted from [Mike Ash](https://mikeash.com/pyblog/fluid-simulation-for-dummies.html)

pub static N: i32 = 64;
pub static SCALE: i32 = 10;
pub static ITER: i32 = 4;

pub fn IX(x: i32, y: i32, z: i32) -> usize {
    let x = x.min(N - 1).max(0);
    let y = y.min(N - 1).max(0);
    let z = z.min(N - 1).max(0);

    (x + y * N + z * i32::pow(N, 2)) as usize
}

pub fn unsafe_index(array: &mut Vec<f32>, index: usize, write: f32) {
    unsafe {
        write_volatile(&mut array[index], write);
    }
}

/// Set bounds: prevents fluid from leaking out of the environment (box/window). Counters any
/// outwards pressure with an equal and opposite pressure, to keep the fluid inside.
pub fn set_bnd(b: i32, x: &mut Vec<f32>) {
    for j in 1..(N - 1) {
        for i in 1..(N - 1) {
            x[IX(i, j, 0)] = if b == 3 {
                -x[IX(i, j, 1)]
            } else {
                    x[IX(i, j, 1)]
                };
            x[IX(i, j, N - 1)] = if b == 3 {
                -x[IX(i, j, N - 2)]
            } else {
                    -x[IX(i, j, N - 2)]
                }
        }
    }
    for k in 1..(N - 1) {
        for i in 1..(N - 1) {
            x[IX(i, 0, k)] = if b == 2 {
                -x[IX(i, 1, k)]
            } else {
                    x[IX(i, 1, k)]
                };
            x[IX(i, N - 1, k)] = if b == 2 {
                -x[IX(i, N - 2, k)]
            } else {
                    x[IX(i, N - 2, k)]
                };
        }
    }
    for k in 1..(N - 1) {
        for j in 1..(N - 1) {
            x[IX(0, j, k)] = if b == 1 {
                -x[IX(1, j, k)]
            } else {
                    x[IX(1, j, k)]
                };
            x[IX(N - 1, j, k)] = if b == 1 {
                -x[IX(N - 2, j, k)]
            } else {
                    x[IX(N - 2, j, k)]
                };
        }
    }

    x[IX(0, 0, 0)] = 0.33 * (x[IX(1, 0, 0)] + x[IX(0, 1, 0)] + x[IX(0, 0, 1)]);
    x[IX(0, N - 1, 0)] = 0.33 * (x[IX(1, N - 1, 0)] + x[IX(0, N - 2, 0)] + x[IX(0, N - 1, 1)]);
    x[IX(0, 0, N - 1)] = 0.33 * (x[IX(1, 0, N - 1)] + x[IX(0, 1, N - 1)] + x[IX(0, 0, N)]);
    x[IX(0, N - 1, N - 1)] =
        0.33 * (x[IX(1, N - 1, N - 1)] + x[IX(0, N - 2, N - 1)] + x[IX(0, N - 1, N - 2)]);
    x[IX(N - 1, 0, 0)] = 0.33 * (x[IX(N - 2, 0, 0)] + x[IX(N - 1, 1, 0)] + x[IX(N - 1, 0, 1)]);
    x[IX(N - 1, N - 1, 0)] =
        0.33 * (x[IX(N - 2, N - 1, 0)] + x[IX(N - 1, N - 2, 0)] + x[IX(N - 1, N - 1, 1)]);
    x[IX(N - 1, 0, N - 1)] =
        0.33 * (x[IX(N - 2, 0, N - 1)] + x[IX(N - 1, 1, N - 1)] + x[IX(N - 1, 0, N - 2)]);
    x[IX(N - 1, N - 1, N - 1)] = 0.33
        * (x[IX(N - 2, N - 1, N - 1)] + x[IX(N - 1, N - 2, N - 1)] + x[IX(N - 1, N - 1, N - 2)]);
}

pub fn lin_solve(b: i32, x: &mut Vec<f32>, x0: &Vec<f32>, a: f32, c: f32) {
    let c_recip: f32 = 1.0 / c;
    for _ in 0..ITER {
        for i in 1..(N - 1) {
            for j in 1..(N - 1) {
                for k in 1..(N - 1) {
                    unsafe {
                        write_volatile(&mut x[IX(k,j,i)], (
                            x0[IX(k, j, i)]
                            + (x[IX(k + 1, j, i)]
                            + x[IX(k - 1, j, i)]
                            + x[IX(k, j + 1, i)]
                            + x[IX(k, j - 1, i)]
                            + x[IX(k, j, i + 1)]
                            + x[IX(k, j, i - 1)])
                        ) * c_recip
                        );
                    }
                }
            }
        }
        set_bnd(b, x);
    }
}

// Precalculates a value and passes everything off to lin_solve.
pub fn diffuse(b: i32, x: &mut Vec<f32>, x0: &mut Vec<f32>, diff: f32, dt: f32) {
    let a: f32 = dt * diff * i32::pow(N - 2, 2) as f32;
    lin_solve(b, x, x0, a, 1.0 + 6.0 * a);
}

/// This function is responsible for actually moving things around. To that end, it looks at each
/// cell in turn. In that cell, it grabs the velocity, follows that velocity b ack in time, and
/// sees where it lands. It the ntakes a weighted average of the cells around the spot where it
/// lands, then applies the value to the current cell.
pub fn advect(
    b: i32,
    d: &mut Vec<f32>,
    d0: &Vec<f32>,
    vx: &Vec<f32>,
    vy: &Vec<f32>,
    vz: &Vec<f32>,
    dt: f32,
) {
    let (mut i0, mut i1, mut j0, mut j1, mut k0, mut k1): (f32, f32, f32, f32, f32, f32);
    let (mut s0, mut s1, mut t0, mut t1, mut u0, mut u1): (f32, f32, f32, f32, f32, f32);

    let (mut tmp1, mut tmp2, mut tmp3, mut x, mut y, mut z): (f32, f32, f32, f32, f32, f32);

    let dtx: f32 = dt * (N - 2) as f32;
    let dty: f32 = dt * (N - 2) as f32;
    let dtz: f32 = dt * (N - 2) as f32;

    let n_float: f32 = N as f32;

    let (mut k, mut k_float) = (1, 1.0);
    while k < N - 1 {
        let (mut j, mut j_float) = (1, 1.0);
        while j < N - 1 {
            let (mut i, mut i_float) = (1, 1.0);
            while i < N - 1 {
                tmp1 = dtx * vx[IX(i, j, k)];
                tmp2 = dty * vy[IX(i, j, k)];
                tmp3 = dtz * vz[IX(i, j, k)];
                x = i_float - tmp1;
                y = j_float - tmp2;
                z = k_float - tmp3;

                if x < 0.5 {
                    x = 0.5
                };
                if x > n_float + 0.5 {
                    x = n_float + 0.5
                };
                i0 = x.floor();
                i1 = i0 + 1.0;
                if y < 0.5 {
                    y = 0.5
                };
                if y > n_float + 0.5 {
                    y = n_float + 0.5
                };
                j0 = y.floor();
                j1 = j0 + 1.0;
                if z < 0.5 {
                    z = 0.5
                };
                if z > n_float + 0.5 {
                    z = n_float + 0.5
                };
                k0 = z.floor();
                k1 = k0 + 1.0;

                s1 = x - i0;
                s0 = 1.0 - s1;
                t1 = y - j0;
                t0 = 1.0 - t1;
                u1 = z - k0;
                u0 = 1.0 - u1;

                let i0i: i32 = i0 as i32;
                let i1i: i32 = i1 as i32;
                let j0i: i32 = j0 as i32;
                let j1i: i32 = j1 as i32;
                let k0i: i32 = k0 as i32;
                let k1i: i32 = k1 as i32;

                d[IX(i, j, k)] = s0
                    * (t0 * (u0 * d0[IX(i0i, j0i, k0i)] + u1 * d0[IX(i0i, j0i, k1i)])
                    + (t1 * (u0 * d0[IX(i0i, j1i, k0i)] + u1 * d0[IX(i0i, j1i, k1i)])))
                    + s1 * (t0 * (u0 * d0[IX(i1i, j0i, k0i)] + u1 * d0[IX(i1i, j0i, k1i)])
                    + (t1 * (u0 * d0[IX(i1i, j1i, k0i)] + u1 * d0[IX(i1i, j1i, k1i)])));
                i += 1;
                i_float += 1.0;
            }
            j += 1;
            j_float += 1.0;
        }
        k += 1;
        k_float += 1.0;
    }
    set_bnd(b, d);
}

/// When dealing with *incompressible* fluids, the amount of fluid in each box has to stay constant.
/// This means that the amount of fluid input has to equal the amount of fluid output. Some boxes
/// generate a net inflow/outflow. This function balances everything out.
pub fn project(
    vx: &mut Vec<f32>,
    vy: &mut Vec<f32>,
    vz: &mut Vec<f32>,
    p: &mut Vec<f32>,
    div: &mut Vec<f32>,
) {
    for k in 1..(N - 1) {
        for j in 1..(N - 1) {
            for i in 1..(N - 1) {
                div[IX(i, j, k)] = -0.5
                    * (vx[IX(i + 1, j, k)] - vx[IX(i - 1, j, k)] + vy[IX(i, j + 1, k)]
                    - vy[IX(i, j - 1, k)]
                    + vz[IX(i, j, k + 1)]
                    - vz[IX(i, j, k - 1)])
                    / N as f32;
                p[IX(i, j, k)] = 0.0;
            }
        }
    }
    set_bnd(0, div);
    set_bnd(0, p);
    lin_solve(0, p, div, 1.0, 6.0);

    for k in 1..(N - 1) {
        for j in 1..(N - 1) {
            for i in 1..(N - 1) {
                vx[IX(i, j, k)] -= 0.5 * (p[IX(i + 1, j, k)] - p[IX(i - 1, j, k)]) * N as f32;
                vy[IX(i, j, k)] -= 0.5 * (p[IX(i, j + 1, k)] - p[IX(i, j - 1, k)]) * N as f32;
                vz[IX(i, j, k)] -= 0.5 * (p[IX(i, j, k + 1)] - p[IX(i, j, k - 1)]) * N as f32;
            }
        }
    }
    set_bnd(1, vx);
    set_bnd(2, vy);
    set_bnd(3, vz);
}

pub struct FluidCube {
    size: i32,
    dt: f32,
    diff: f32,
    visc: f32,

    s: Vec<f32>,
    density: Vec<f32>,

    vx: Vec<f32>,
    vy: Vec<f32>,
    vz: Vec<f32>,

    vx0: Vec<f32>,
    vy0: Vec<f32>,
    vz0: Vec<f32>,
}

impl FluidCube {
    pub fn new(dt: f32, diffusion: f32, viscosity: f32) -> Self {
        let def = Vec::with_capacity(i32::pow(N, 3) as usize);
        Self {
            dt,
            size: N as i32,
            diff: diffusion,
            visc: viscosity,

            s: def.clone(),
            density: def.clone(),

            vx: def.clone(),
            vy: def.clone(),
            vz: def.clone(),

            vx0: def.clone(),
            vy0: def.clone(),
            vz0: def.clone(),
        }
    }

    /// Add density refers to the density of the "dye", not the fluid
    pub fn add_density(&mut self, x: i32, y: i32, z: i32, amount: f32) {
        unsafe {
            write_volatile(
                &mut self.density[IX(x,y,z)],
                read_volatile(&self.density[IX(x, y, z)] as *const f32) + amount
            );
        }
    }

    pub fn add_velocity(
        &mut self,
        x: i32,
        y: i32,
        z: i32,
        amount_x: f32,
        amount_y: f32,
        amount_z: f32,
    ) {
        unsafe {
            // I'm so sorry
            let mut a = self.density[IX(x,y,z)];
            let sum = &mut &mut (a + amount_x + amount_y + amount_z) as *const &mut f32;
            write_volatile(&mut &mut a, read_volatile(sum));
        }
    }

    pub fn step(&mut self) {
        diffuse(1, &mut self.vx0, &mut self.vx, self.visc, self.dt);
        diffuse(2, &mut self.vy0, &mut self.vy, self.visc, self.dt);
        diffuse(3, &mut self.vz0, &mut self.vz, self.visc, self.dt);

        println!("step!!!!!!! asdfhasdfhjkasdfhjkl");
        project(
            &mut self.vx0,
            &mut self.vy0,
            &mut self.vz0,
            &mut self.vx,
            &mut self.vy,
        );

        advect(
            1,
            &mut self.vx,
            &self.vx0,
            &self.vx0,
            &self.vy0,
            &self.vz0,
            self.dt,
        );
        advect(
            2,
            &mut self.vy,
            &self.vy0,
            &self.vx0,
            &self.vy0,
            &self.vz0,
            self.dt,
        );
        advect(
            3,
            &mut self.vz,
            &self.vz0,
            &self.vx0,
            &self.vy0,
            &self.vz0,
            self.dt,
        );

        project(
            &mut self.vx,
            &mut self.vy,
            &mut self.vz,
            &mut self.vx0,
            &mut self.vy0,
        );
        diffuse(0, &mut self.s, &mut self.density, self.diff, self.dt);
        advect(
            0,
            &mut self.density,
            &self.s,
            &self.vx,
            &self.vy,
            &self.vz,
            self.dt,
        );
    }

    pub fn render(&mut self, dh: &mut RaylibDrawHandle) {
        for i in 0..N {
            for j in 0..N {
                for k in 0..N {
                    let x: i32 = i * SCALE;
                    let y: i32 = j * SCALE;
                    let z: i32 = k * SCALE;

                    // let d: f32 = self.density[IX(i, j, k)];
                    dh.draw_rectangle(x, y, SCALE, SCALE, Color::WHITESMOKE);
                }
            }
        }
    }
}
