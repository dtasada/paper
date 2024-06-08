use raylib::prelude::*;

mod engine;
use engine::*;

fn main() -> Result<(), String> {
    let (mut rl, thread) = raylib::init().size(1280, 720).build();
    rl.set_target_fps(60);

    // let mut particle = Particle::new(Vector2::new(rl.get_screen_width() as f32 / 2.0, rl.get_screen_height() as f32 / 2.0), Vector2::zero(), 50.0);
    let mut bounds = Rectangle::new(
        20.0,
        20.0,
        rl.get_screen_width() as f32 - 40.0,
        rl.get_screen_height() as f32 - 40.0,
    );

    let mut particles: Vec<Particle> = vec![];

    for i in 1..=20 {
        particles.push(Particle::new(
            Vector2::new(bounds.x + i as f32 * 50.0, bounds.y + 50.0),
            Vector2::zero(),
            50.0,
        ));
    }

    while !rl.window_should_close() {
        let mut dh = rl.begin_drawing(&thread);
        dh.clear_background(Color::BLACK);

        for particle in &mut particles {
            particle.vel.y += GRAVITY;

            particle.pos += particle.vel * dh.get_frame_time();
            particle.collision(&dh, &bounds);

            dh.draw_circle(
                particle.pos.x as i32,
                particle.pos.y as i32,
                particle.radius,
                Color::LIGHTBLUE,
            );
        }
        dh.draw_fps(12, 12);
        dh.draw_rectangle_lines_ex(bounds, 1.0, Color::ORANGE);
    }

    Ok(())
}