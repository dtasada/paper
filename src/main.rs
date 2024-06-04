use raylib::prelude::*;

mod engine;
use engine::*;

fn main() -> Result<(), String> {
    let (mut rl, thread) = raylib::init().size(1280, 720).build();

    let mut fluid = FluidCube::new(0.1, 0.0, 0.0);

    while !rl.window_should_close() {
        let mut dh = rl.begin_drawing(&thread);

        dh.clear_background(Color::BLACK);
        dh.draw_fps(24, 24);
        fluid.step();
    }

    Ok(())
}
