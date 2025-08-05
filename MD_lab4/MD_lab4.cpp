#include <iostream>
#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <stdexcept>

using namespace std;

// Typ definicji dla macierzy komórek
using CellGrid = vector<vector<int>>;

// Funkcja do inicjalizacji macierzy
CellGrid initializeGrid(int rows, int cols) {
    return CellGrid(rows, vector<int>(cols, 0)); // Inicjalizacja martwych komórek
}

// Funkcja do inicjalizacji komórek
CellGrid initializeRandomCells(int rows, int cols, double liveCellProbability) {
    CellGrid cells(rows, vector<int>(cols, 0));
    srand(static_cast<unsigned int>(time(0)));

    // Losowe ustawienie komórek
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cells[i][j] = (static_cast<double>(rand()) / RAND_MAX < liveCellProbability) ? 1 : 0;
        }
    }
    return cells;
}

// Funkcja do umieszczania glidera w centrum macierzy
void placeGlider(CellGrid& grid) {
    int rows = grid.size();
    int cols = grid[0].size();

    // Obliczanie pozycji, aby umieścić glider w centrum
    int startRow = (rows - 3) / 2;
    int startCol = (cols - 3) / 2;

    // Ustawianie żywych komórek glidera
    grid[startRow][startCol + 1] = 1; // (0, 1)
    grid[startRow + 1][startCol + 2] = 1; // (1, 2)
    grid[startRow + 2][startCol] = 1; // (2, 0)
    grid[startRow + 2][startCol + 1] = 1; // (2, 1)
    grid[startRow + 2][startCol + 2] = 1; // (2, 2)
}

void placeToad(CellGrid& grid) {
    int rows = grid.size();
    int cols = grid[0].size();

    // Obliczanie pozycji, aby umieścić Toad w centrum
    int startRow = (rows - 2) / 2;
    int startCol = (cols - 6) / 2;

    // Ustawianie żywych komórek dla Toad
    grid[startRow][startCol] = 1; 
    grid[startRow][startCol + 1] = 1;
    grid[startRow][startCol + 2] = 1; 
    grid[startRow + 1][startCol + 1] = 1; 
    grid[startRow + 1][startCol + 2] = 1; 
    grid[startRow + 1][startCol + 3] = 1; 
}

// Funkcja do inicjalizacji niezmiennego stanu (blok)
CellGrid initializeStableBlock(int rows, int cols) {
    // Ustawiamy komórki na martwe
    CellGrid cells(rows, vector<int>(cols, 0));

    // Obliczanie środkowych indeksów
    int centerRow = rows / 2;
    int centerCol = cols / 2;

    // Definiujemy blok jako niezmienny stan
    // Ustawiamy żywe komórki w bloku 2x2
    if (rows >= 2 && cols >= 2) {
        cells[centerRow - 1][centerCol - 1] = 1; // Ustawiamy żywe komórki
        cells[centerRow - 1][centerCol] = 1;
        cells[centerRow][centerCol - 1] = 1;
        cells[centerRow][centerCol] = 1;
    }

    return cells;
}

void drawCells(sf::RenderWindow& window, const CellGrid& cells) {
    int cellSize = 5; // Rozmiar pojedynczej komórki
    sf::RectangleShape cell(sf::Vector2f(cellSize - 1, cellSize - 1)); 
    for (int i = 0; i < cells.size(); ++i) {
        for (int j = 0; j < cells[i].size(); ++j) {
            cell.setPosition(j * cellSize, i * cellSize); // Ustal pozycję komórki
            if (cells[i][j] == 1) {
                cell.setFillColor(sf::Color::Green); // Kolor żywej komórki
            }
            else {
                cell.setFillColor(sf::Color::Black); // Kolor martwej komórki
            }
            window.draw(cell); // Rysowanie komórki
        }
    }
}

int countAliveNeighborsReflecting(const CellGrid& cells, int row, int col) {
    int aliveNeighbors = 0;
    int rows = cells.size();
    int cols = cells[0].size();

    // Sprawdzenie sąsiadów w 8 kierunkach
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            if (x == 0 && y == 0) continue; // Pomijamy siebie

            // Obliczamy indeksy sąsiadów z uwzględnieniem granic odbijających
            int ni = row + x;
            int nj = col + y;

            // Odbijanie się od granic
            if (ni < 0) ni = -ni; // Odbicie od górnej granicy
            if (ni >= rows) ni = 2 * rows - ni - 1; // Odbicie od dolnej granicy
            if (nj < 0) nj = -nj; // Odbicie od lewej granicy
            if (nj >= cols) nj = 2 * cols - nj - 1; // Odbicie od prawej granicy

            aliveNeighbors += cells[ni][nj]; // Dodajemy sąsiada
        }
    }

    return aliveNeighbors;
}

// Funkcja do zliczania żywych sąsiadów z uwzględnieniem warunków brzegowych periodycznych
int countAliveNeighborsPeriodic(const CellGrid& cells, int row, int col) {
    int aliveNeighbors = 0;
    int rows = cells.size();
    int cols = cells[0].size();

    // Sprawdzenie sąsiadów w 8 kierunkach
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            if (x == 0 && y == 0) continue; // Pomijamy siebie

            // Obliczamy indeksy sąsiadów z uwzględnieniem granic periodycznych
            int ni = (row + x + rows) % rows; // Użycie operatora modulo
            int nj = (col + y + cols) % cols; // Użycie operatora modulo

            aliveNeighbors += cells[ni][nj]; // Dodajemy sąsiada
        }
    }

    return aliveNeighbors;
}

// Funkcja do aktualizacji komórek zgodnie z zasadami gry w życie z uwzględnieniem reguły
void updateCells(CellGrid& cells, int rule, bool isReflecting) {
    int rows = cells.size();
    int cols = cells[0].size();
    CellGrid newCells = cells; // Tworzenie nowej macierzy dla nowych wartości

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Zliczanie żywych sąsiadów
            int aliveNeighbors;
            // Zliczamy żywych sąsiadów w zależności od wybranego warunku
            if (isReflecting) {
                aliveNeighbors = countAliveNeighborsReflecting(cells, i, j);
            }
            else {
                aliveNeighbors = countAliveNeighborsPeriodic(cells, i, j);
            }

            if (rule == 40)
            {
                if (cells[i][j] == 1) { // Komórka żywa
                    if (aliveNeighbors < 2 || aliveNeighbors > 3) {
                        newCells[i][j] = 0; // Umiera
                    }
                }
                else { // Komórka martwa
                    if (aliveNeighbors == 3) {
                        newCells[i][j] = 1; // Ożywa
                    }
                }
            }
            else if (rule == 63)
            {
                if (cells[i][j] == 1) { // Komórka żywa
                    // Żywa komórka umiera, jeśli ma 0, 1, lub 4-8 żywych sąsiadów
                    if (aliveNeighbors == 0 || aliveNeighbors == 1 || aliveNeighbors >= 4) {
                        newCells[i][j] = 0; // Umiera
                    }
                }
                else { // Komórka martwa
                    // Martwa komórka ożywa, jeśli ma dokładnie 2 lub 3 żywych sąsiadów
                    if (aliveNeighbors == 2 || aliveNeighbors == 3) {
                        newCells[i][j] = 1; // Ożywa
                    }
                }
            }
            else if (rule == 26)
            {
                if (cells[i][j] == 1) { // Komórka żywa
                    // Żywa komórka umiera, jeśli ma 0, 1, 4, 5, 6, 7 lub 8 żywych sąsiadów
                    if (aliveNeighbors == 0 || aliveNeighbors == 1 || aliveNeighbors == 4 || aliveNeighbors == 5 || aliveNeighbors == 6 || aliveNeighbors == 7 || aliveNeighbors == 8) {
                        newCells[i][j] = 0; // Umiera
                    }
                }
                else { // Komórka martwa
                    // Martwa komórka ożywa, jeśli ma dokładnie 3 żywych sąsiadów
                    if (aliveNeighbors == 3) {
                        newCells[i][j] = 1; // Ożywa
                    }
                }
            }
            else if (rule == 190)
            {
                if (cells[i][j] == 1) { // Komórka żywa
                    newCells[i][j] = 0; // Żywa komórka umiera
                }
                else { // Komórka martwa
                    // Martwa komórka ożywa, jeśli ma dokładnie 2 żywych sąsiadów
                    if (aliveNeighbors == 2) {
                        newCells[i][j] = 1; // Ożywa
                    }
                }
            }
        }
    }
    cells = newCells; 
}

void updateWindowTitle(sf::RenderWindow& window, int rule) {
    window.setTitle("Regula: " + std::to_string(rule));
}

int main() {
    int rows = 200, cols = 200, steps = 20; 
    vector<int> rules = { 40, 63, 26, 190 }; 
    int currentRuleIndex = 0; 
    bool isReflecting = false;

    try {
        CellGrid cells = initializeGrid(rows, cols);

        // Glider
        placeGlider(cells);

        // Oscylator
        //placeToad(cells);

        // Losowe
       // cells = initializeRandomCells(rows, cols, 0.05);

        // Niezmienny
       // cells = initializeStableBlock(rows, cols);

        // Tworzenie okna SFML
        sf::RenderWindow window(sf::VideoMode(cols * 5, rows * 5), "Automat Komorkowy - Gra w Zycie - Reguła: " + std::to_string(rules[currentRuleIndex]));

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close(); 
            }

            //cout << "Aktualna reguła: " << rules[currentRuleIndex] << endl;

            for (int step = 0; step < steps; ++step) {
                updateCells(cells, rules[currentRuleIndex], isReflecting); 
                window.clear(); 
                drawCells(window, cells); 

                updateWindowTitle(window, rules[currentRuleIndex]);

                window.display();

            }

            currentRuleIndex = (currentRuleIndex + 1) % rules.size();
            // cells = initializeCells(rows, cols); // Inicjalizacja nowych komórek
        }
    }
    catch (const exception& e) {
        cerr << "Wystapil blad: " << e.what() << endl;
        return 1;
    }

    return 0;
}
