import pygame
import random
import time

TILE_SIZE = 1

Healthy = 0
Burning = 1
Burned = 2
Water = 3
Empty = 4

class Tree:
    def __init__(self):
        self.state = Healthy
        self.color = (0, 255, 0)  # drzewo

    def ignite(self):
        if self.state == Healthy:
            self.state = Burning
            self.color = (255, 0, 0)  # ogień

    def burn(self):
        if self.state == Burning:
            self.state = Burned
            self.color = (0, 0, 0)  # spalone

    def extinguish(self):
        if self.state == Burning:
            self.state = Healthy
            self.color = (34, 139, 34)  # zdrowe


def initialize_forest_from_image(forest, image):
    for y in range(image.get_height()):
        for x in range(image.get_width()):
            pixel_color = image.get_at((x, y))

            # drzewo
            if pixel_color.g > pixel_color.r and pixel_color.g > pixel_color.b and pixel_color.g > 100:
                if pixel_color.g > 150:
                    forest[y][x].state = Healthy
                    forest[y][x].color = (34, 139, 34)
                elif pixel_color.g > 120:
                    forest[y][x].state = Healthy
                    forest[y][x].color = (50, 205, 50)
                else:
                    forest[y][x].state = Healthy
                    forest[y][x].color = (0, 128, 0)  #
            # spalone
            elif pixel_color.r > 80 and pixel_color.g < 50 and pixel_color.b < 30:
                forest[y][x].state = Burned
                forest[y][x].color = (0, 0, 0)
            # woda
            elif pixel_color == (138, 216, 236) or pixel_color == (122, 213, 233):
                forest[y][x].state = Water
                forest[y][x].color = pixel_color
            # puste
            else:
                forest[y][x].state = Empty
                forest[y][x].color = (255, 255, 255)

    forest[image.get_height() // 2][image.get_width() // 2].ignite()


def spread_fire(forest, humidity, wind):
    fire_spreading = []

    for y in range(len(forest)):
        for x in range(len(forest[y])):
            if forest[y][x].state == Burning:
                fire_spreading.append((y, x))

    newly_ignited = []

    # rozprzestrzenianie ognia na sąsiednie drzewa
    for fire in fire_spreading:
        y, x = fire

        # sąsiednie komórki w zależności od wiatru
        directions = [(dy, dx) for dy in range(-1, 2) for dx in range(-1, 2) if (dy != 0 or dx != 0)]
        random.shuffle(directions)

        # wpływ wiatru
        wind_directions = [(dy + wind[0], dx + wind[1]) for dy, dx in directions]

        for dy, dx in wind_directions:
            ny, nx = y + dy, x + dx

            if 0 <= ny < len(forest) and 0 <= nx < len(forest[0]):
                current_tree = forest[ny][nx]

                # puste
                if current_tree.state == Empty:
                    continue

                # spalone
                if current_tree.state == Burned:
                    continue

                # zdrowe
                if current_tree.state == Healthy:
                    chance = 70  # szansa na zapalenie

                    if current_tree.color == (50, 205, 50):  # jasny zielony
                        chance = 90
                    elif current_tree.color == (34, 139, 34):  # ciemny zielony
                        chance = 60

                    chance = chance - int(humidity * 0.2)

                    if random.randint(0, 99) < chance:
                        current_tree.ignite()
                        newly_ignited.append((ny, nx))

        forest[y][x].burn()


def draw_forest(window, forest):
    for y in range(len(forest)):
        for x in range(len(forest[y])):
            pygame.draw.rect(window, forest[y][x].color, (x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE))


def initialize_environment():
    humidity = random.randint(0, 50)

    wind = (random.choice([-1, 0, 1]), random.choice([-1, 0, 1]))

    return humidity, wind


def update_environment(humidity, wind):
    humidity = random.randint(0, 100)
    wind = (random.choice([-1, 0, 1]), random.choice([-1, 0, 1]))

    return humidity, wind


def extinguish_trees_in_area(forest, mouse_x, mouse_y, extinguish_range=20):
    for y in range(mouse_y - extinguish_range // 2, mouse_y + extinguish_range // 2):
        for x in range(mouse_x - extinguish_range // 2, mouse_x + extinguish_range // 2):
            if 0 <= y < len(forest) and 0 <= x < len(forest[0]):
                if forest[y][x].state == Burning:
                    forest[y][x].extinguish()


def main():
    pygame.init()

    image = pygame.image.load('mapaa1.png')

    original_width = image.get_width()
    original_height = image.get_height()

    new_width = original_width // 2
    new_height = original_height // 2
    image = pygame.transform.scale(image, (new_width, new_height))

    window_width = new_width * TILE_SIZE
    window_height = new_height * TILE_SIZE

    window = pygame.display.set_mode((window_width, window_height))
    pygame.display.set_caption("Symulacja pożaru lasu")

    # Inicjalizacja lasu
    forest = [[Tree() for _ in range(new_width)] for _ in range(new_height)]
    initialize_forest_from_image(forest, image)

    # Inicjalizacja warunków środowiskowych
    humidity, wind = initialize_environment()

    clock = pygame.time.Clock()
    running = True
    last_update_time = time.time()

    while running:
        window.fill((255, 255, 255))

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            # detekcja kliknięcia myszy (gaszenie ognia)
            if event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:
                    mouse_x, mouse_y = event.pos
                    grid_x, grid_y = mouse_x // TILE_SIZE, mouse_y // TILE_SIZE

                    extinguish_trees_in_area(forest, grid_x, grid_y, extinguish_range=40)

        # zmiana wiatru i wilgotności
        if time.time() - last_update_time > 5:
            humidity, wind = update_environment(humidity, wind)
            last_update_time = time.time()

        spread_fire(forest, humidity, wind)

        draw_forest(window, forest)

        pygame.display.update()
        clock.tick(10)  # aktualizacja co 10 klatek na sekundę

    pygame.quit()


if __name__ == '__main__':
    main()
