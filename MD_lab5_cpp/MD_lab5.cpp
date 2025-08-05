#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>

// Stałe rozmiary
const int TILE_SIZE = 1;  // Rozmiar pojedynczego kwadratu w pikselach

// Stany drzewa
enum TreeState { Healthy, Burning, Burned, Water, Empty };

// Struktura reprezentująca drzewo
struct Tree {
    TreeState state;
    sf::Color color;

    Tree() : state(Healthy), color(sf::Color::Green) {}

    void ignite() {
        if (state == Healthy) {
            state = Burning;
            color = sf::Color::Red; // Kolor ognia
        }
    }

    void burn() {
        if (state == Burning) {
            state = Burned;
            color = sf::Color::Black; // Spalone drzewo
        }
    }
};

// Funkcja do wczytania obrazu i stworzenia mapy terenu
void initializeForestFromImage(std::vector<std::vector<Tree>>& forest, const sf::Image& image) {
    for (int y = 0; y < image.getSize().y; ++y) {
        for (int x = 0; x < image.getSize().x; ++x) {
            sf::Color pixelColor = image.getPixel(x, y);

            // Jeśli kolor jest głównie zielony (drzewo)
            if (pixelColor.g > pixelColor.r && pixelColor.g > pixelColor.b && pixelColor.g > 100) {
                // Dodajemy różne odcienie zielonego
                if (pixelColor.g > 150) {
                    forest[y][x].state = Healthy;
                    forest[y][x].color = sf::Color(34, 139, 34);  // Ciemniejszy zielony
                }
                else if (pixelColor.g > 120) {
                    forest[y][x].state = Healthy;
                    forest[y][x].color = sf::Color(50, 205, 50);  // Jaśniejszy zielony
                }
                else {
                    forest[y][x].state = Healthy;
                    forest[y][x].color = sf::Color(0, 128, 0);  // Mniej intensywny zielony
                }
            }
            // Jeśli kolor jest brązowy (spalony teren)
            else if (pixelColor.r > 80 && pixelColor.g < 50 && pixelColor.b < 30) {
                forest[y][x].state = Burned;
                forest[y][x].color = sf::Color::Black;
            }
            // Jeśli kolor jest jednym z odcieni wody
            else if ((pixelColor.r == 138 && pixelColor.g == 216 && pixelColor.b == 236) ||
                (pixelColor.r == 122 && pixelColor.g == 213 && pixelColor.b == 233)) {
                forest[y][x].state = Water;
                forest[y][x].color = pixelColor;  // Kolor wody z obrazu (może być jeden z dwóch)
            }
            // Jeśli kolor jest inny (np. pusty teren)
            else {
                forest[y][x].state = Empty;
                forest[y][x].color = sf::Color::White;
            }
        }
    }

    // Rozpal środek
    forest[image.getSize().y / 2][image.getSize().x / 2].ignite();
}

// Funkcja do rozprzestrzeniania ognia
void spreadFire(std::vector<std::vector<Tree>>& forest) {
    std::vector<std::pair<int, int>> fireSpreading;

    // Zbieramy wszystkie drzewa, które się palą
    for (int y = 0; y < forest.size(); ++y) {
        for (int x = 0; x < forest[y].size(); ++x) {
            if (forest[y][x].state == Burning) {
                fireSpreading.push_back({ y, x });
            }
        }
    }

    // Zbiór komórek, które zostały zapalone w tej turze
    std::vector<std::pair<int, int>> newlyIgnited;

    // Rozprzestrzenianie ognia na sąsiednie drzewa
    for (auto& fire : fireSpreading) {
        int y = fire.first;
        int x = fire.second;

        // Sprawdź sąsiednie komórki (góra, dół, lewo, prawo)
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dy == 0 && dx == 0) continue; // Pomijamy siebie

                int ny = y + dy;
                int nx = x + dx;

                if (ny >= 0 && ny < forest.size() && nx >= 0 && nx < forest[0].size()) {
                    if (forest[ny][nx].state == Healthy && forest[ny][nx].state != Water) {
                        // 70% szans, że ogień się rozprzestrzeni, ale nie na wodzie
                        if (rand() % 100 < 70) {
                            forest[ny][nx].ignite();
                            newlyIgnited.push_back({ ny, nx });
                        }
                    }
                }
            }
        }

        // Spal drzewa
        forest[y][x].burn();
    }
}

// Funkcja do rysowania lasu
void drawForest(sf::RenderWindow& window, const std::vector<std::vector<Tree>>& forest) {
    for (int y = 0; y < forest.size(); ++y) {
        for (int x = 0; x < forest[y].size(); ++x) {
            sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            tile.setPosition(x * TILE_SIZE, y * TILE_SIZE);
            tile.setFillColor(forest[y][x].color);
            window.draw(tile);
        }
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // Inicjalizacja generatora losowego

    // Wczytanie obrazu
    sf::Image image;
    if (!image.loadFromFile("mapaa1.png")) {
        std::cerr << "Błąd podczas wczytywania obrazu!" << std::endl;
        return -1;
    }

    // Zmniejszenie rozdzielczości obrazu o połowę
    sf::Image scaledImage;
    scaledImage.create(image.getSize().x / 2, image.getSize().y / 2);
    for (unsigned int y = 0; y < scaledImage.getSize().y; ++y) {
        for (unsigned int x = 0; x < scaledImage.getSize().x; ++x) {
            sf::Color color = image.getPixel(x * 2, y * 2);  // Wybieramy piksel co 2. (w przypadku zmniejszenia o połowę)
            scaledImage.setPixel(x, y, color);
        }
    }

    // Tworzenie okna SFML
    sf::RenderWindow window(sf::VideoMode(scaledImage.getSize().x * TILE_SIZE, scaledImage.getSize().y * TILE_SIZE), "Symulacja pożaru lasu");

    // Inicjalizacja lasu na podstawie obrazu
    std::vector<std::vector<Tree>> forest(scaledImage.getSize().y, std::vector<Tree>(scaledImage.getSize().x));
    initializeForestFromImage(forest, scaledImage);

    // Pętla główna
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Rozprzestrzenianie ognia
        spreadFire(forest);

        // Rysowanie lasu
        window.clear();
        drawForest(window, forest);
        window.display();

        // Brak opóźnienia - symulacja działa jak najszybciej
    }

    return 0;
}
