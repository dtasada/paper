from pyengine.pgbasics import *


WIDTH, HEIGHT = 800, 600
WIN = Window(size=(WIDTH, HEIGHT), title="PAPER")
REN = Renderer(WIN)

m = 200
base_color = (140, 140, 140, 255)
outline_color = (0, 0, 0, 255)
plane = Crystal(
    REN, [
        [-m, -m, m],
        [m, -m, m],
        [m, m, m],
        [-m, m, m],
        [-m, -m, -m],
        [m, -m, -m],
        [m, m, -m],
        [-m, m, -m],
    ], [
        
    ], [

    ], [
        [[base_color, outline_color], 0, 1, 2, 3],
        [[base_color, outline_color], 4, 5, 1, 0],
        [[base_color, outline_color], 4, 5, 6, 7],
        [[base_color, outline_color], 7, 6, 2, 3],
        [[base_color, outline_color], 4, 0, 3, 7],
        [[base_color, outline_color], 1, 5, 6, 2]
    ],
    (WIDTH / 2, HEIGHT / 2), 1, 0, 0, 0, 0.015, 0.015, 0.015,
    fill_as_connections=False
)

clock = pygame.time.Clock()
running = True
while running:
    clock.tick(120)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    fill_rect(REN, (230, 230, 230, 255), (0, 0, WIDTH, HEIGHT))
    plane.update()
    REN.present()

pygame.quit()
sys.exit()
