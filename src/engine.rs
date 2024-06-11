use raylib::prelude::*;
use std::{
    ops::Deref,
    sync::{Arc, Mutex, MutexGuard},
};

/// # paper-engine
// asdf

type GridCell = Vec<Option<Arc<Mutex<Particle>>>>;

pub static GRAVITY: f32 = 9.81;

pub fn get_sign(number: f32) -> f32 {
    if number > 0.0 {
        1.0
    } else if number < 0.0 {
        -1.0
    } else {
        0.0
    }
}

#[derive(Debug)]
pub struct Grid {
    /// width and height are the width and height of each cell
    pub width: i32,
    pub height: i32,
    // Arc of two-dimensional arraylist that contains cells
    pub content: Arc<Mutex<Vec<Vec<GridCell>>>>,
}

impl Grid {
    pub fn new() -> Self {
        let width = 20;
        let height = 20;

        Self {
            width,
            height,
            content: Arc::new(Mutex::new(vec![])),
        }
    }

    /// Finds collisions with adjacent cells
    pub fn find_collisions(&mut self) {
        // x and y start at one to skip topmost and bottommost rows, and leftmost and rightmost columns
        for x in 1..=self.width {
            for y in 1..=self.height {
                for dx in -1..=1 {
                    for dy in -1..=1 {
                        self.solve_collision((x, y), (x + dx, y + dy));
                    }
                }
            }
        }
    }

    fn solve_collision(
        &mut self,
        (cell_a_x, cell_a_y): (i32, i32),
        (cell_b_x, cell_b_y): (i32, i32),
    ) {
        let cell_a = &mut self.content.lock().unwrap()[cell_a_x as usize][cell_a_y as usize];
        let cell_b = &mut self.content.lock().unwrap()[cell_b_x as usize][cell_b_y as usize];

        for particle_a_wrapped in cell_a.iter_mut() {
            for particle_b_wrapped in cell_b.iter_mut() {
                if !std::ptr::eq(particle_a_wrapped, particle_b_wrapped) {
                    if let Some(a_refcell) = particle_a_wrapped.to_owned() {
                        if let Some(b_refcell) = particle_b_wrapped.to_owned() {
                            let mut a = a_refcell.lock().unwrap();
                            let mut b = b_refcell.lock().unwrap();

                            if check_collision_circles(a.pos, a.radius, b.pos, b.radius) {
                                let compensation_length =
                                    a.radius + b.radius - a.pos.distance_to(b.pos);
                                a.pos -= compensation_length / 2.0;
                                b.pos += compensation_length / 2.0;
                            }
                        }
                    }
                }
            }
        }
    }
}

#[derive(Debug, Clone)]
pub struct Particle {
    pub pos: Vector2,
    pub vel: Vector2,
    pub radius: f32,
    collision_damping: f32,
}

impl Particle {
    pub fn new(start_pos: Vector2, vel: Vector2, radius: f32) -> Self {
        Self {
            pos: start_pos,
            vel,
            radius,
            collision_damping: 0.8,
        }
    }

    pub fn collision(&mut self, bounds: &Rectangle) {
        let bounds_size = Vector2::new(bounds.x + bounds.width, bounds.y + bounds.height)
            - Vector2::one() * self.radius;

        if self.pos.x > bounds_size.x {
            self.pos.x = bounds_size.x * get_sign(self.pos.x);
            self.vel.x *= -1.0 * self.collision_damping;
        }

        if self.pos.y > bounds_size.y {
            self.pos.y = bounds_size.y * get_sign(self.pos.y);
            self.vel.y *= -1.0 * self.collision_damping;
        }
    }

    fn in_cell(&self, grid: &Grid, cell: (i32, i32)) -> bool {
        if self.pos.x > (grid.width * cell.0) as f32
            && self.pos.x < (grid.width * cell.0 + grid.width) as f32
            && self.pos.y > (grid.width * cell.1 + grid.height) as f32
            && self.pos.y < (grid.width * cell.1) as f32
        {
            true
        } else {
            false
        }
    }

    pub fn update(&mut self, dh: &mut RaylibDrawHandle, bounds: &Rectangle, grid: &mut Grid) {
        self.vel.y += GRAVITY;

        self.pos += self.vel * dh.get_frame_time();
        self.collision(&bounds);

        let g = grid.content.to_owned();

        for (i, row) in g.lock().unwrap().iter_mut().enumerate() {
            for (j, c) in row.iter_mut().enumerate() {
                let mut indices: Vec<usize> = vec![];
                let cell = Arc::new(Mutex::new(c));
                for (k, p) in cell.lock().unwrap().iter().enumerate() {
                    if let Some(arc_mutex) = p {
                        if let Ok(mut particle) = arc_mutex.lock() {
                            let particle_ref = &mut *particle;
                            let particle_addr = particle_ref as *mut Particle;
                            let self_addr = self as *mut Particle;

                            if particle_addr == self_addr {
                                indices.push(k);
                            }
                        }
                    }

                    // Mutate self in grid
                    for &k in indices.iter().rev() {
                        g.lock().unwrap()[(self.pos.y / grid.height as f32) as usize]
                            [(self.pos.x / grid.width as f32) as usize]
                            .push(cell.lock().unwrap().swap_remove(k));
                    }
                }
            }

            dh.draw_circle(
                self.pos.x as i32,
                self.pos.y as i32,
                self.radius,
                Color::LIGHTBLUE,
            );
        }
    }
}
