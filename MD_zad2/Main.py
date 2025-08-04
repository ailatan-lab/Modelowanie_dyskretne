import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Parametry modelu
grid_size = (100, 100)  # Rozmiar siatki (100x100)
tau = 1.0  # Czas relaksacji
num_steps = 300  # Liczba kroków czasowych
w = [0.25, 0.25, 0.25, 0.25]  # Wagi dla D2Q4 (wszystkie kierunki równoważne)
dx = 1.0  # Odległość między węzłami
dt = 1.0  # Krok czasowy

# Wektory prędkości dla D2Q4
c = np.array([[1, 0], [-1, 0], [0, 1], [0, -1]])

# Inicjalizacja funkcji rozkładu
f_in = np.zeros((*grid_size, 4))  # Funkcje wejściowe (4 kierunki)
f_out = np.zeros_like(f_in)  # Funkcje wyjściowe

# Inicjalizacja stężenia (C) i przeszkód
C = np.zeros(grid_size)
C[:, :grid_size[1] // 2] = 1.0  # Wypełniona lewa połowa siatki

# Funkcja rozkładu początkowego: inicjalizowanie f_in na podstawie równowagowego rozkładu
for i in range(4):
    f_in[..., i] = w[i] * C  # Funkcje wejściowe inicjowane jako równowagowe

# Przeszkoda - ściana z otworem
obstacle = np.zeros(grid_size, dtype=bool)  # Przeszkoda
obstacle[40:60, 50] = True  # Ściana z otworem
obstacle[49:51, 50] = False  # Otwór w ścianie

# Funkcja kolizji
def collision(f_in, tau):
    C_local = np.sum(f_in, axis=-1)  # Wyznaczenie stężenia
    f_eq = np.zeros_like(f_in)  # Rozkłady równowagowe
    for i in range(4):
        f_eq[..., i] = w[i] * C_local  # Równowagowy rozkład na podstawie stężenia
    f_out = f_in + (dt / tau) * (f_eq - f_in)  # Relaksacja
    return f_out

# Funkcja streaming
def streaming(f_out):
    f_in = np.zeros_like(f_out)
    for i, ci in enumerate(c):
        f_in[..., i] = np.roll(f_out[..., i], shift=ci, axis=(0, 1))  # Przemieszczanie funkcji rozkładu
    return f_in

# Warunki brzegowe (bounce-back)
def apply_boundary_conditions(f_in, obstacle):
    for i, ci in enumerate(c):
        opposite = (i + 1) % 2 if i < 2 else (i + 1) % 2 + 2  # Wyznaczenie przeciwnego kierunku
        f_in[..., i][obstacle] = f_in[..., opposite][obstacle]  # Odbicie funkcji rozkładu na przeszkodach
    return f_in

# Przygotowanie wykresu
fig, ax = plt.subplots()
im = ax.imshow(C, cmap='hot', origin='lower', vmin=0, vmax=1)
plt.colorbar(im, ax=ax, label='Stężenie')
plt.title('Symulacja dyfuzji (LBM)')
plt.xlabel('x')
plt.ylabel('y')

# Funkcja aktualizacji do animacji
def update(step):
    global f_in
    global C
    # Operacja kolizji
    f_out = collision(f_in, tau)

    # Operacja streaming
    f_in = streaming(f_out)

    # Warunki brzegowe
    f_in = apply_boundary_conditions(f_in, obstacle)

    # Wyznaczenie stężenia
    C = np.sum(f_in, axis=-1)

    # Aktualizacja obrazu
    im.set_array(C)
    return [im]

# Tworzenie animacji
anim = FuncAnimation(fig, update, frames=num_steps, interval=50, blit=True)
plt.show()
