use raylib::prelude::*;
use std::cell::RefCell;
use std::rc::Rc;

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

    let mut particles: Vec<Rc<RefCell<Particle>>> = Vec::new();

    // Generate particles
    for i in 1..=grid.height {
        grid.content.push(vec![]); // Create row
        for j in 1..=grid.width {
            grid.content[(i - 1) as usize].push(vec![]); // Create Column
            let mut new_particle = Particle::new(
                Vector2::new(bounds.x + j as f32 * 50.0, bounds.y + i as f32 * 50.0),
                Vector2::zero(),
                20.0,
            );

            let p = Rc::new(RefCell::new(new_particle));
            particles.push(p.clone());
            grid.content[(i - 1) as usize][(j - 1) as usize].push(Some(p));
        }
    }
    
    while !rl.window_should_close() {
        let mut dh = rl.begin_drawing(&thread);
        dh.clear_background(Color::BLACK);

        for particle in &mut particles {
            particle.borrow_mut().update(&mut dh, &bounds, &mut grid);
        }

        grid.find_collisions();

        dh.draw_fps(12, 12);
        dh.draw_rectangle_lines_ex(bounds, 1.0, Color::ORANGE);
    }

    Ok(())
}
