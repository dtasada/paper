use raylib::prelude::*;

#[macro_use]
mod engine;
use engine::*;

fn main() -> Result<(), String> {
    let (mut rl, thread) = raylib::init().size(1280, 720).build();
    
    let fluid = FluidCube::new(0.1, 0.0, 0.0);

    while !rl.window_should_close() {
        let mut d = rl.begin_drawing(&thread);

        d.clear_background(Color::BLACK);
        d.draw_fps(24, 24);
    }

    Ok(())
}
