from pygame._sdl2.video import Window, Renderer, Texture
from math import floor
import pygame
import cProfile


pygame.init()

N = 128
R = 10
iter_ = 16

window = Window("cfd", (N*R, N*R))
renderer = Renderer(window)


def write(surf, anchor, text, font, color, x, y, alpha=255, blit=True, border=None, special_flags=0, tex=False, ignore=True):
    if ignore:
        # return
        pass
    if border is not None:
        bc, bw = border, 1
        write(surf, anchor, text, font, bc, x - bw, y - bw, special_flags=special_flags),
        write(surf, anchor, text, font, bc, x + bw, y - bw, special_flags=special_flags),
        write(surf, anchor, text, font, bc, x - bw, y + bw, special_flags=special_flags),
        write(surf, anchor, text, font, bc, x + bw, y + bw, special_flags=special_flags)
    text = font.render(str(text), True, color)
    if tex:
        text = Texture.from_surface(surf, text)
        text.alpha = alpha
    else:
        text.set_alpha(alpha)
    text_rect = text.get_rect()
    setattr(text_rect, anchor, (int(x), int(y)))
    if blit:
        surf.blit(text, text_rect, special_flags=special_flags)
    return text, text_rect