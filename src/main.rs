use raylib::prelude::*;

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

    let mut particles: Vec<Particle> = Vec::new();

    // Generate particles
    for i in 1..=grid.width {
        for j in 1..=grid.height {
            particles.push(Particle::new(
                Vector2::new(bounds.x + i as f32 * 50.0, bounds.y + j as f32 * 50.0),
                Vector2::zero(),
                20.0,
            ));
        }
    }

    while !rl.window_should_close() {
        let mut dh = rl.begin_drawing(&thread);
        dh.clear_background(Color::BLACK);

        for particle in &mut particles {
            particle.update(&mut dh, &bounds);
        }

        grid.find_collisions();

        dh.draw_fps(12, 12);
        dh.draw_rectangle_lines_ex(bounds, 1.0, Color::ORANGE);
    }

    Ok(())
}
