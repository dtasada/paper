use raylib::prelude::*;

mod engine;
use engine::*;

fn main() -> Result<(), String> {
    let (mut rl, thread) = raylib::init().size(N * SCALE, N * SCALE).build();

    let mut fluid = FluidCube::new(0.1, 0.0, 0.0);

    log::log!(log::Level::Info, "Starting application...");

    while !rl.window_should_close() {
        let mut dh = rl.begin_drawing(&thread);

        // fluid.step();

        dh.clear_background(Color::BLACK);
        dh.draw_fps(24, 24);

        if dh.get_mouse_delta().x != 0.0 || dh.get_mouse_delta().y != 0.0 {
            fluid.add_density(dh.get_mouse_x() / SCALE, dh.get_mouse_y() / SCALE, 0, 100.0);
            fluid.add_velocity(
                dh.get_mouse_x() / SCALE,
                dh.get_mouse_y() / SCALE,
                0, // z
                dh.get_mouse_delta().x,
                dh.get_mouse_delta().y,
                0.0, // z amount
            );
        }

        // fluid.render(&mut dh);
    }

    Ok(())
}
