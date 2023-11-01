from pyengine.pgbasics import *


WIDTH, HEIGHT = 800, 600
WIN = Window(size=(WIDTH, HEIGHT), title="PAPER")
REN = Renderer(WIN)


vertices = []

for _ in range(8):
    vertices.append([randf(-1, 1), randf(-1, 1), randf(-1, 1)])
y = 0
g = 0.1
m = 1
base_color = lambda: rand_rgba()
outline_color = (0, 0, 0, 255)
plane = Crystal(
    REN, [
        # [-m, -m, m],
        # [m, -m, m],
        # [m, m, m],
        # [-m, m, m],
        # [-m, -m, -m],
        # [m, -m, -m],
        # [m, m, -m],
        # [-m, m, -m],
        *vertices
    ], [

    ], [

    ], [
        [[base_color(), outline_color], rand(0, 7), rand(0, 7), rand(0, 7)],
        [[base_color(), outline_color], rand(0, 7), rand(0, 7), rand(0, 7)],
        [[base_color(), outline_color], rand(0, 7), rand(0, 7), rand(0, 7)],
        [[base_color(), outline_color], rand(0, 7), rand(0, 7), rand(0, 7)],
        [[base_color(), outline_color], rand(0, 7), rand(0, 7), rand(0, 7)],
        [[base_color(), outline_color], rand(0, 7), rand(0, 7), rand(0, 7)],
        [[base_color(), outline_color], rand(0, 7), rand(0, 7), rand(0, 7)],
        [[base_color(), outline_color], rand(0, 7), rand(0, 7), rand(0, 7)],
        [[base_color(), outline_color], rand(0, 7), rand(0, 7), rand(0, 7)],
    ],
    (WIDTH / 2, HEIGHT / 2), 200, 0, 0, 0, 0, 0, 0,
    fill_as_connections=False
)

clock = pygame.time.Clock()
running = True
while running:
    mouses = pygame.mouse.get_pressed()
    clock.tick(120)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        elif event.type == pygame.MOUSEMOTION:
            if mouses[0]:
                dx, dy = event.rel
                m = 0.015
                plane.ya -= dx * 0.01
                plane.xa += dy * 0.01

    fill_rect(REN, (230, 230, 230, 255), (0, 0, WIDTH, HEIGHT))
    plane.update()
    REN.present()

pygame.quit()
sys.exit()
