from fluid import *
import pygame

font = pygame.font.Font(None, 40)
clock = pygame.time.Clock()


def main():
    running = True
    pmousex = pygame.mouse.get_pos()[0]
    pmousey = pygame.mouse.get_pos()[1]

    while running:
        clock.tick(0)
        renderer.draw_color = (0, 0, 0)
        renderer.fill_rect(pygame.Rect(0, 0, N * R, N * R))

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            if pygame.mouse.get_pressed()[0]:
                fluid.add_density(
                    int(pygame.mouse.get_pos()[0] / R),
                    int(pygame.mouse.get_pos()[1] / R),
                    100,
                )
                amt_x = pygame.mouse.get_pos()[0] - pmousex
                amt_y = pygame.mouse.get_pos()[1] - pmousey
                fluid.add_velocity(
                    int(pygame.mouse.get_pos()[0] / R),
                    int(pygame.mouse.get_pos()[1] / R),
                    amt_x,
                    amt_y,
                )

        fluid.step()
        fluid.render_d()
        write(
            renderer,
            text=f"{clock.get_fps()}",
            anchor="topleft",
            color=(255, 255, 255),
            font=font,
            x=0,
            y=0,
            tex=True,
        )

        pmousex = pygame.mouse.get_pos()[0]
        pmousey = pygame.mouse.get_pos()[1]

        renderer.present()

    pygame.quit()


# main()
cProfile.run("main()", sort="cumtime")

