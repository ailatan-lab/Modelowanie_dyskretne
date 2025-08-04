import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

grid_size = (128, 128)
tau = 0.6
num_steps = 100000
w = [4 / 9] + [1 / 9] * 4 + [1 / 36] * 4
c = np.array([
    [0, 0], [1, 0], [-1, 0], [0, 1], [0, -1],
    [1, 1], [-1, -1], [-1, 1], [1, -1]
])

f_in = np.zeros((*grid_size, 9))
rho = np.ones(grid_size) * 1.0
ux = np.zeros(grid_size)
uy = np.zeros(grid_size)

obstacle = np.zeros(grid_size, dtype=bool)
obstacle[:, 30] = True
obstacle[45:65, 30] = False

for i in range(9):
    cu = c[i, 0] * ux + c[i, 1] * uy
    f_in[..., i] = w[i] * rho * (1 + 3 * cu + 4.5 * cu ** 2 - 1.5 * (ux ** 2 + uy ** 2))

def collision(f_in, tau):
    rho_local = np.sum(f_in, axis=-1)
    rho_local[rho_local == 0] = 1e-6
    ux_local = np.sum(f_in * c[:, 0], axis=-1) / rho_local
    uy_local = np.sum(f_in * c[:, 1], axis=-1) / rho_local
    f_eq = np.zeros_like(f_in)
    for i in range(9):
        cu = c[i, 0] * ux_local + c[i, 1] * uy_local
        f_eq[..., i] = w[i] * rho_local * (1 + 3 * cu + 4.5 * cu ** 2 - 1.5 * (ux_local ** 2 + uy_local ** 2))
    f_out = f_in + (1 / tau) * (f_eq - f_in)
    return f_out

def streaming(f_out):
    f_in = np.zeros_like(f_out)
    for i, ci in enumerate(c):
        f_in[..., i] = np.roll(f_out[..., i], shift=ci[::-1], axis=(0, 1))
    return f_in

def apply_boundary_conditions(f_in, obstacle, grid_size):
    opposite = [0, 1, 2, 3, 4, 5, 6, 7, 8]
    opposite[1], opposite[2] = 2, 1
    opposite[3], opposite[4] = 4, 3
    opposite[5], opposite[8] = 8, 5
    opposite[6], opposite[7] = 7, 6

    for i in range(9):
        f_in[..., i][obstacle] = f_in[..., opposite[i]][obstacle]

    # Warunki na górnej i dolnej granicy
    ux[0, :] = 0.02  # Górna granica (wyżej niż na dolnej)
    ux[-1, :] = 0.0  # Dolna granica

    # Liniowa zmiana prędkości ux na lewych i prawych granicach
    ux[:, 0] = np.linspace(0.0, 0.02, grid_size[0])  # Lewa granica
    ux[:, -1] = np.linspace(0.0, 0.02, grid_size[0])  # Prawa granica

    # Aktualizacja warunków brzegowych
    for i in [3, 5, 6, 7, 8]:
        f_in[0, :, i] = f_in[0, :, opposite[i]]
    for i in [4, 5, 6, 7, 8]:
        f_in[-1, :, i] = f_in[-1, :, opposite[i]]
    for i in [1, 5, 6, 7, 8]:
        f_in[:, 0, i] = f_in[:, 0, opposite[i]]
    for i in [2, 5, 6, 7, 8]:
        f_in[:, -1, i] = f_in[:, -1, opposite[i]]

    # Obsługa górnej i dolnej granicy
    f_in[0, :, [1, 5, 6, 7, 8]] = f_in[1, :, [1, 5, 6, 7, 8]]  # Symetryczny warunek na górze
    f_in[-1, :, [3, 5, 6, 7, 8]] = f_in[-2, :, [3, 5, 6, 7, 8]]  # Bounce-back na dole

    # Obsługa prawej i lewej granicy
    wall_column = 30
    open_start = 45
    open_end = 65

    for i in range(9):
        f_in[:, wall_column, i][(np.arange(grid_size[0]) < open_start) | (np.arange(grid_size[0]) > open_end)] = \
        f_in[:, wall_column + 1, opposite[i]][(np.arange(grid_size[0]) < open_start) | (np.arange(grid_size[0]) > open_end)]

    left_wall_column = 0
    for i in range(9):
        f_in[:, left_wall_column, i] = f_in[:, left_wall_column + 1, opposite[i]]

    return f_in

fig, ax = plt.subplots(1, 2, figsize=(12, 6))
im_ux1 = ax[0].imshow(ux, cmap='RdBu', origin='lower', vmin=-0.01, vmax=0.01)
ax[0].set_title("Prędkość pozioma")
plt.colorbar(im_ux1, ax=ax[0])

im_uy2 = ax[1].imshow(uy, cmap='RdBu', origin='lower', vmin=-0.01, vmax=0.01)
ax[1].set_title("Prędkość pionowa")
plt.colorbar(im_uy2, ax=ax[1])

convergence_threshold = 1e-4
max_no_change_steps = 20

no_change_count = 0
prev_ux, prev_uy = np.zeros_like(ux), np.zeros_like(uy)

source_velocity_x = 0.5
source_velocity_y = 0.5
source_position = (52, 12)

# Dodaj parametr na początku symulacji
source_duration = 10  # Ilość iteracji, w których źródło działa

def update(step):
    global f_in, rho, ux, uy, prev_ux, prev_uy, no_change_count
    print(f"Iteracja: {step}")
    rho[:, 30:] = 1.0
    rho[:, :30] = 0.95
    ux[:, :] = 0
    uy[:, :] = 0

    # Źródło cząstek działa tylko przez określoną liczbę kroków
    if step < source_duration:
        ux[source_position] = source_velocity_x
        uy[source_position] = source_velocity_y

        for i in range(9):
            cu = c[i, 0] * ux[source_position] + c[i, 1] * uy[source_position]
            f_in[..., i][source_position] = w[i] * rho[source_position] * (
                1 + 3 * cu + 4.5 * cu ** 2 - 1.5 * (ux[source_position] ** 2 + uy[source_position] ** 2)
            )

    f_out = collision(f_in, tau)

    f_in = streaming(f_out)

    f_in = apply_boundary_conditions(f_in, obstacle, grid_size)

    rho = np.sum(f_in, axis=-1)

    rho[rho == 0] = 1e-6

    ux_new = np.sum(f_in * c[:, 0], axis=-1) / rho
    uy_new = np.sum(f_in * c[:, 1], axis=-1) / rho

    max_ux_diff = np.max(np.abs(ux_new - prev_ux))
    max_uy_diff = np.max(np.abs(uy_new - prev_uy))

    if max_ux_diff < convergence_threshold and max_uy_diff < convergence_threshold:
        no_change_count += 1
    else:
        no_change_count = 0

    if no_change_count >= max_no_change_steps:
        print(f"Symulacja zakończona po {step} krokach (osiągnięcie stanu stacjonarnego).")
        anim.event_source.stop()

    prev_ux[:] = ux_new
    prev_uy[:] = uy_new

    ux[:] = ux_new
    uy[:] = uy_new

    im_ux1.set_array(ux)
    im_uy2.set_array(uy)

    return [im_ux1, im_uy2]


anim = FuncAnimation(fig, update, frames=num_steps, interval=30, blit=True)
plt.show()
