from fluid_utils import *


class Fluid:
    def __init__(self, dt, diff, visc):
        self.dt = dt
        self.diff = diff
        self.visc = visc

        self.s = [0 for _ in range(N*N)]
        self.density = [0 for _ in range(N*N)]

        self.vx = [0 for _ in range(N*N)]
        self.vy = [0 for _ in range(N*N)]

        self.vx0 = [0 for _ in range(N*N)]
        self.vy0 = [0 for _ in range(N*N)]

    def add_density(self, x, y, amount):
        index = IX(x, y)
        self.density[index] += amount

    def add_velocity(self, x, y, amount_x, amount_y):
        index = IX(x, y)
        self.vx[index] += amount_x
        self.vy[index] += amount_y

    def step(self):
        visc = self.visc
        diff = self.diff
        dt = self.dt 
        vx = self.vx
        vy  = self.vy
        vx0  = self.vx0
        vy0  = self.vy0
        s = self.s
        density = self.density 
        
        diffuse(1, vx0, vx, visc, dt)
        diffuse(2, vy0, vy, visc, dt)
        
        project(vx0, vy0, vx, vy)
        
        advect(1, vx, vx0, vx0, vy0, dt)
        advect(2, vy, vy0, vx0, vy0, dt)
        
        project(vx, vy, vx0, vy0)
        
        diffuse(0, s, density, diff, dt)
        advect(0, density, s, vx, vy, dt)

    def render_d(self):
        for i in range(N):
            for j in range(N):
                x = i * R
                y = j * R
                d = int(self.density[IX(i, j)])
                if d > 255:
                    d = 255
                renderer.draw_color = (d, d,d)
                renderer.fill_rect(pygame.Rect(x, y, R, R))


fluid = Fluid(0.05, 0, 0)


