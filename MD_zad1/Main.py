import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Parametry symulacji
grid_size = 100
num_particles = 5000
num_iterations = 200
directions = np.array([(-1, 0), (1, 0), (0, -1), (0, 1)])

# Przeszkody
grid = np.zeros((grid_size, grid_size), dtype=int)
grid[0:, 30] = -1
grid[45:65, 30] = 0

# Cząstki (nierównomierny rozkład)
in_grid = np.zeros((grid_size, grid_size, 4), dtype=int)

# Zdefiniowanie obszarów o większej liczbie cząstek
dense_area = (slice(20, 50), slice(5, 25))  # Obszar o dużej koncentracji cząstek

for _ in range(num_particles):
    # Losowanie pozycji cząstek
    if np.random.rand() < 0.7:  # Większe prawdopodobieństwo w gęstszym obszarze
        row = np.random.randint(*dense_area[0].indices(grid_size))
        col = np.random.randint(*dense_area[1].indices(grid_size))
    else:  # Pozostałe cząstki w losowych miejscach
        row = np.random.randint(0, grid_size)
        col = np.random.randint(0, 30)

    direction = np.random.randint(0, 4)
    in_grid[row, col, direction] = 1


def streaming():
    global in_grid
    out_grid = np.zeros_like(in_grid)
    for i in range(grid_size):
        for j in range(grid_size):
            for dir in range(4):
                if in_grid[i, j, dir] == 1:
                    ni, nj = i + directions[dir][0], j + directions[dir][1]
                    if 0 <= ni < grid_size and 0 <= nj < grid_size and grid[ni, nj] != -1:
                        out_grid[ni, nj, dir] = 1
                    else:
                        if dir == 0:
                            out_grid[i, j, 1] = 1
                        elif dir == 1:
                            out_grid[i, j, 0] = 1
                        elif dir == 2:
                            out_grid[i, j, 3] = 1
                        elif dir == 3:
                            out_grid[i, j, 2] = 1
    in_grid = out_grid


def collision():
    global in_grid
    out_grid = np.zeros_like(in_grid)
    for i in range(grid_size):
        for j in range(grid_size):
            if in_grid[i, j, 0] == 1 and in_grid[i, j, 1] == 1:
                out_grid[i, j, 2] = 1
                out_grid[i, j, 3] = 1
            elif in_grid[i, j, 2] == 1 and in_grid[i, j, 3] == 1:
                out_grid[i, j, 0] = 1
                out_grid[i, j, 1] = 1
            else:
                out_grid[i, j] = in_grid[i, j]
    in_grid = out_grid


def update(frame):
    streaming()
    collision()
    display_grid = np.zeros((grid_size, grid_size), dtype=int)
    for i in range(grid_size):
        for j in range(grid_size):
            if np.any(in_grid[i, j]):
                display_grid[i, j] = 1
    for i in range(grid_size):
        for j in range(grid_size):
            if grid[i, j] == -1:
                display_grid[i, j] = -1
    mat.set_array(display_grid)
    return [mat]


fig, ax = plt.subplots(figsize=(6, 6))
ax.axis("off")
display_grid = np.zeros((grid_size, grid_size), dtype=int)
mat = ax.matshow(display_grid, cmap="gray", vmin=-1, vmax=1)
ani = FuncAnimation(fig, update, frames=num_iterations, interval=0, blit=True)
plt.show()
