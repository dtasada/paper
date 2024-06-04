use raylib::prelude::*;

pub static N: i32 = 256;
pub static ITER: i32 = 10;

pub enum State {
    Menu,
    Play,
    SettingsMenu,
    SettingsPlay,
}

pub fn IX(x: i32, y: i32, z: i32) -> usize {
    (x + y * N + z * i32::pow(N, 2)) as usize
}

pub fn set_bnd(b: i32, x: &mut Vec<f32>) {
    for j in 1..N {
        for i in 1..N {
            x[IX(i, j, 0  )] = if b == 3 {
                -x[IX(i, j, 1)] 
            } else {
                    x[IX(i, j, 1  )]
                };
            x[IX(i, j, N-1)] = if b == 3 {
                -x[IX(i, j, N-2)] 
            } else {
                    -x[IX(i, j, N-2)] 
                }
        }
    }
    for k in 1..N {
        for i in 1..N {
            x[IX(i, 0  , k)] = if b == 2 { -x[IX(i, 1  , k)] } else { x[IX(i, 1  , k)] };
            x[IX(i, N-1, k)] = if b == 2 { -x[IX(i, N-2, k)] } else { x[IX(i, N-2, k)]};
        }
    }
    for k in 1..N {
        for j in 1..N {
            x[IX(0  , j, k)] = if b == 1 { -x[IX(1  , j, k)] } else { x[IX(1  , j, k)]};
            x[IX(N-1, j, k)] = if b == 1  {-x[IX(N-2, j, k)]} else {x[IX(N-2, j, k)]};
        }
    }
    
    x[IX(0, 0, 0)]       = 0.33 * (x[IX(1, 0, 0)]
                                 + x[IX(0, 1, 0)]
                                 + x[IX(0, 0, 1)]);
    x[IX(0, N-1, 0)]     = 0.33 * (x[IX(1, N-1, 0)]
                                 + x[IX(0, N-2, 0)]
                                 + x[IX(0, N-1, 1)]);
    x[IX(0, 0, N-1)]     = 0.33 * (x[IX(1, 0, N-1)]
                                 + x[IX(0, 1, N-1)]
                                 + x[IX(0, 0, N)]);
    x[IX(0, N-1, N-1)]   = 0.33 * (x[IX(1, N-1, N-1)]
                                 + x[IX(0, N-2, N-1)]
                                 + x[IX(0, N-1, N-2)]);
    x[IX(N-1, 0, 0)]     = 0.33 * (x[IX(N-2, 0, 0)]
                                 + x[IX(N-1, 1, 0)]
                                 + x[IX(N-1, 0, 1)]);
    x[IX(N-1, N-1, 0)]   = 0.33 * (x[IX(N-2, N-1, 0)]
                                 + x[IX(N-1, N-2, 0)]
                                 + x[IX(N-1, N-1, 1)]);
    x[IX(N-1, 0, N-1)]   = 0.33 * (x[IX(N-2, 0, N-1)]
                                 + x[IX(N-1, 1, N-1)]
                                 + x[IX(N-1, 0, N-2)]);
    x[IX(N-1, N-1, N-1)] = 0.33 * (x[IX(N-2, N-1, N-1)]
                                 + x[IX(N-1, N-2, N-1)]
                                 + x[IX(N-1, N-1, N-2)]);
}

pub fn lin_solve(b: i32, x: Vec<f32>, x0: Vec<f32>, a: f32, c: f32) {
    let c_repic: f32 = 1.0 / c;
    for k in 0..ITER {
        for m in 1..N {
            for j in 1..N {
                for i in 1..N {
                    x[IX(i, j, m)] =
                        (x0[IX(i, j, m)]
                            + a*(    x[IX(i+1, j  , m  )]
                                    +x[IX(i-1, j  , m  )]
                                    +x[IX(i  , j+1, m  )]
                                    +x[IX(i  , j-1, m  )]
                                    +x[IX(i  , j  , m+1)]
                                    +x[IX(i  , j  , m-1)]
                           )) * c_repic;
                }
            }
        }
        set_bnd(b, &mut x);
    }
}

pub fn diffuse(b: i32, x: Vec<f32>, x0: Vec<f32>, diff: f32, dt: f32) {
    let a: f32 = dt * diff * i32::pow(N - 2, 2) as f32;
    lin_solve(b, x, x0, a, 1.0 + 6.0 * a);
}

pub fn advect(b: i32, d: Vec<f32>, d0: Vec<f32>, velocX: Vec<f32>, velocY: Vec<f32>, velocZ: Vec<f32>, dt: f32) {
    let (i0, i1, j0, j1, k0, k1): (f32, f32, f32, f32, f32, f32);
    
    let dtx: f32 = dt * (N - 2) as f32;
    let dty: f32 = dt * (N - 2) as f32;
    let dtz: f32 = dt * (N - 2) as f32;
    
    let (s0, s1, t0, t1, u0, u1): (f32, f32, f32, f32, f32, f32);
    let (tmp1, tmp2, tmp3, x, y, z): (f32, f32, f32, f32, f32, f32);
    
    let Nfloat: f32 = N as f32;
    let (ifloat, jfloat, kfloat): (f32, f32, f32);
    let (i, j, k): (i32, i32, i32);
    
    (k, kfloat) = (1, 1.0);
    (j, jfloat) = (1, 1.0);
    (i, ifloat) = (1, 1.0);
    while k < N - 1 {

    
    
    }
    for(k = 1, kfloat = 1; k < N - 1; k++, kfloat++) {
        for(j = 1, jfloat = 1; j < N - 1; j++, jfloat++) { 
            for(i = 1, ifloat = 1; i < N - 1; i++, ifloat++) {
                tmp1 = dtx * velocX[IX(i, j, k)];
                tmp2 = dty * velocY[IX(i, j, k)];
                tmp3 = dtz * velocZ[IX(i, j, k)];
                x    = ifloat - tmp1; 
                y    = jfloat - tmp2;
                z    = kfloat - tmp3;
                
                if(x < 0.5f) x = 0.5f; 
                if(x > Nfloat + 0.5f) x = Nfloat + 0.5f; 
                i0 = floorf(x); 
                i1 = i0 + 1.0f;
                if(y < 0.5f) y = 0.5f; 
                if(y > Nfloat + 0.5f) y = Nfloat + 0.5f; 
                j0 = floorf(y);
                j1 = j0 + 1.0f; 
                if(z < 0.5f) z = 0.5f;
                if(z > Nfloat + 0.5f) z = Nfloat + 0.5f;
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
                
                    s0 * ( t0 * (u0 * d0[IX(i0i, j0i, k0i)]
                                +u1 * d0[IX(i0i, j0i, k1i)])
                        +( t1 * (u0 * d0[IX(i0i, j1i, k0i)]
                                +u1 * d0[IX(i0i, j1i, k1i)])))
                   +s1 * ( t0 * (u0 * d0[IX(i1i, j0i, k0i)]
                                +u1 * d0[IX(i1i, j0i, k1i)])
                        +( t1 * (u0 * d0[IX(i1i, j1i, k0i)]
                                +u1 * d0[IX(i1i, j1i, k1i)])));
            }
        }
    }
    set_bnd(b, d, N);
}

pub fn project(vx: &mut Vec<f32>, vy: &mut Vec<f32>, vz: &mut Vec<f32>, p: Vec<f32>, mut div: Vec<f32>) {
    for k in 1..N {
        for j in 1..N {
            for i in 1..N {
                div[IX(i, j, k)] = -0.5*(
                         vx[IX(i+1, j  , k  )]
                        -vx[IX(i-1, j  , k  )]
                        +vy[IX(i  , j+1, k  )]
                        -vy[IX(i  , j-1, k  )]
                        +vz[IX(i  , j  , k+1)]
                        -vz[IX(i  , j  , k-1)]
                    )/N as f32;
                p[IX(i, j, k)] = 0.0;
            }
        }
    }
    set_bnd(0, &mut div); 
    set_bnd(0, &mut p);
    lin_solve(0, p, div, 1.0, 6.0);
    
    for k in 1..N {
        for j in 1..N {
            for i in 1..N {
                vx[IX(i, j, k)] -= 0.5 * (  p[IX(i+1, j, k)]
                                               -p[IX(i-1, j, k)]) * N as f32;
                vy[IX(i, j, k)] -= 0.5 * (  p[IX(i, j+1, k)]
                                               -p[IX(i, j-1, k)]) * N as f32;
                vz[IX(i, j, k)] -= 0.5 * (  p[IX(i, j, k+1)]
                                                -p[IX(i, j, k-1)]) * N as f32;
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
            dt: dt,
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

    pub fn add_density(mut self, x: i32, y: i32, z: i32, amount: f32) {
        self.density[IX(x, y, z)] += amount;
    }

    pub fn add_velocity(mut self, x: i32, y: i32, z: i32, amount_x: f32, amount_y: f32, amount_z: f32) {
        let i = IX(x, y, z);

        self.density[i] += amount_x;
        self.density[i] += amount_y;
        self.density[i] += amount_z;
    }

    pub fn step(mut self) {
        diffuse(1, self.vx0, self.vx, self.visc, self.dt);
        diffuse(2, self.vy0, self.vy, self.visc, self.dt);
        diffuse(3, self.vz0, self.vz, self.visc, self.dt);

        project(self.vx0, self.vy0, self.vz0, self.vx, self.vy);

        advect(1, self.vx, self.vx0, self.vx0, self.vy0, self.vz0, self.dt);
        advect(2, self.vy, self.vy0, self.vx0, self.vy0, self.vz0, self.dt);
        advect(3, self.vz, self.vz0, self.vx0, self.vy0, self.vz0, self.dt);

        project(self.vx, self.vy, self.vz, self.vx0, self.vy0);

        diffuse(0, self.s, self.density, self.diff, self.dt);
        advect(0, self.density, self.s, self.vx, self.vy, self.vz, self.dt);
    }
}
