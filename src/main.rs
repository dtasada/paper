use raylib::prelude::*;
use std::sync::{Arc, Mutex};

mod engine;
use engine::*;

fn main() -> Result<(), String> {
    let (mut rl, thread) = raylib::init().size(1280, 720).build();
    rl.set_target_fps(60);

    // Box in which particles are allowed
    let mut bounds = Rectangle::new(
        20.0,
        20.0,
        rl.get_screen_width() as f32 - 40.0,
        rl.get_screen_height() as f32 - 40.0,
    );

    let mut grid = Grid::new();

    let mut particles: Vec<Arc<Mutex<Particle>>> = Vec::new();

    // Generate particles
    {
        let g = Arc::new(&mut grid);
        for i in 1..=g.height {
            g.clone().content.lock().unwrap().push(vec![]); // Create row
            for j in 1..=g.width {
                g.content.lock().unwrap()[(i - 1) as usize].push(vec![]); // Create Column
                let new_particle = Particle::new(
                    Vector2::new(bounds.x + j as f32 * 50.0, bounds.y + i as f32 * 50.0),
                    Vector2::zero(),
                    20.0,
                );

                let p = Arc::new(Mutex::new(new_particle));
                particles.push(p.clone());
                g.clone().content.lock().unwrap()[(i - 1) as usize][(j - 1) as usize].push(Some(p));
            }
        }
    }

    while !rl.window_should_close() {
        let mut dh = rl.begin_drawing(&thread);
        dh.clear_background(Color::BLACK);

        // println!("particles: {:?}", particles);
        for particle in &mut particles {
            particle.lock().unwrap().update(&mut dh, &bounds, &mut grid);
        }

        grid.find_collisions();

        dh.draw_fps(12, 12);
        dh.draw_rectangle_lines_ex(bounds, 1.0, Color::ORANGE);
    }

    Ok(())
}
