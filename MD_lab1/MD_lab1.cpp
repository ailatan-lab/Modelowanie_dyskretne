#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp> // SFML dla wczytania obrazów
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

// Funkcja pomocnicza do ładowania tekstury do OpenGL
int ust(const std::string& filePath)
{
    // Ustawienia kontekstu OpenGL
    sf::ContextSettings settings;
    settings.depthBits = 24;       // Głębia 24-bitowa
    settings.stencilBits = 8;      // Szablon bitowy
    settings.majorVersion = 4;     // OpenGL wersja główna
    settings.minorVersion = 6;     // OpenGL wersja pomocnicza (4.6)

    // Tworzenie okna z powyższymi ustawieniami
    sf::Window window(sf::VideoMode(800, 600), "SFML + OpenGL", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    // Inicjalizacja GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Nie można zainicjalizować GLEW!" << std::endl;
        return -1;
    }

    // Ładowanie obrazu i tworzenie tekstury
    sf::Image image;
    if (!image.loadFromFile(filePath)) {
        std::cerr << "Nie można wczytać obrazu: " << filePath << std::endl;
        return -1;
    }
    image.flipVertically();

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());

    glBindTexture(GL_TEXTURE_2D, 0);

    // Sprawdzenie poprawności tekstury
    if (textureID == 0) {
        std::cerr << "Blad podczas tworzenia tekstury!" << std::endl;
        return -1;
    }

    // Pętla renderowania
    while (window.isOpen()) {
        // Obsługa zdarzeń
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Czyszczenie ekranu
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Włączenie tekstur
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Rysowanie kwadratu z teksturą
        glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f); glVertex2f(-0.5f, -0.5f);
        glTexCoord2f(1.f, 0.f); glVertex2f(0.5f, -0.5f);
        glTexCoord2f(1.f, 1.f); glVertex2f(0.5f, 0.5f);
        glTexCoord2f(0.f, 1.f); glVertex2f(-0.5f, 0.5f);
        glEnd();

        // Odwiązanie tekstury
        glBindTexture(GL_TEXTURE_2D, 0);

        // Wyświetlenie obrazu na ekranie
        window.display();
    }

    // Usuwanie tekstury po zamknięciu aplikacji
    glDeleteTextures(1, &textureID);

    return 0;
}


// Funkcja do wczytania macierzy z pliku tekstowego
std::vector<std::vector<int>> loadMatrixFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    std::vector<std::vector<int>> matrix;
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku: " << filePath << std::endl;
        return matrix;
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<int> row;
        int value;

        while (ss >> value) {
            row.push_back(value);
        }
        matrix.push_back(row);
    }

    return matrix;
}

void saveMatrixToFile(const std::string& filePath, const std::vector<std::vector<int>>& matrix) {
    std::ofstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku: " << filePath << std::endl;
        return;
    }

    for (const auto& row : matrix) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << " ";  // Dodajemy spację między wartościami
            }
        }
        file << "\n";  // Nowa linia po każdym wierszu macierzy
    }

    file.close();

    if (file) {
        std::cout << "Macierz zostala zapisana do pliku: " << filePath << std::endl;
    }
    else {
        std::cerr << "Wystapil blad podczas zapisywania pliku: " << filePath << std::endl;
    }
}

int saveImageToFile(vector<vector<int>> matrix, string filePath3)
{
    int rows = matrix.size();
    int cols = matrix[0].size();

    sf::Image image;
    image.create(cols, rows, sf::Color::Black);

    // Wypełniamy obraz danymi z macierzy
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int value = matrix[y][x];
            // Sprawdzenie, czy wartość jest w zakresie 0-255
            if (value < 0 || value > 255) {
                std::cerr << "Blad: Wartosc " << value << " jest poza zakresem [0, 255]!" << std::endl;
                return -1;
            }

            // Tworzenie koloru w odcieniach szarości
            sf::Color color(value, value, value);
            image.setPixel(x, y, color);
        }
    }

    // Zapisz obraz do pliku
    if (!image.saveToFile(filePath3)) {
        std::cerr << "Nie mozna zapisac obrazu do pliku." << std::endl;
        return -1;
    }

    std::cout << "Obraz zostal pomyslnie zapisany jako " << filePath3 << std::endl;
}

int sciemnianie(int b, string filePath, string filePath2, string filePath3)
{
    // Wczytaj macierz z pliku tekstowego
    std::vector<std::vector<int>> matrix = loadMatrixFromFile(filePath);
    vector<vector <int>> matrix2 = loadMatrixFromFile(filePath);

    if (matrix.empty()) {
        std::cerr << "Blad podczas wczytywania macierzy." << std::endl;
        return -1;
    }

    // Uzyskaj wymiary macierzy
    int rows = matrix.size();
    int cols = matrix[0].size();

    // Tworzymy obraz o wymiarach macierzy
    

    // Zmiana pliku txt
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            int i = matrix2[y][x];
            int new_value = i * (1.0 - b / 100.0);

            if (new_value < 0)
                new_value = 0;
            if (new_value > 255)
                new_value = 255;

            matrix2[y][x] = new_value;
        }
    }

    saveMatrixToFile(filePath2, matrix2);
    saveImageToFile(matrix2, filePath3);
}

int binaryzacja(int b, string filePath, string filePath2, string filePath3)
{
    // Wczytaj macierz z pliku tekstowego
    std::vector<std::vector<int>> matrix = loadMatrixFromFile(filePath);


    if (matrix.empty()) {
        std::cerr << "Blad podczas wczytywania macierzy." << std::endl;
        return -1;
    }

    // Uzyskaj wymiary macierzy
    int rows = matrix.size();
    int cols = matrix[0].size();

    // Tworzymy obraz o wymiarach macierzy
    sf::Image image;
    image.create(cols, rows, sf::Color::Black);

    int bin = 255.0 * (b / 100.0);
    cout << "bin " << bin << endl;
    // Zmiana pliku txt
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            int i = matrix[y][x];

            if (i >= bin)
                i = 255;
            else 
                i = 0;

            matrix[y][x] = i;
        }
    }

    saveMatrixToFile(filePath2, matrix);
    saveImageToFile(matrix, filePath3);
}

int main() 
{
    

    string filePath = "Mapa_MD_no_terrain_low_res_Gray.txt";
    string filePath2 = "output.txt";
    string filePath3 = "output.bmp";
    sciemnianie(90, filePath, filePath2, filePath3);
    ust(filePath3);
    sciemnianie(0, filePath, filePath2, filePath3);
    ust(filePath3);
    sciemnianie(-90, filePath, filePath2, filePath3);
    ust(filePath3);
    

    //rozjasnianie
    int k = -20;
    int b = 20;
    for (int i = 0; i<3; i++)
    {
        sciemnianie(k,filePath, filePath2, filePath3);
        ust(filePath3);
        k = k * (1.0 + b / 100.0);
    }

    //zad3
    binaryzacja(84, filePath, filePath2, filePath3);
    ust(filePath3);

    //zad4
    b = 0;
    while (b != -1)
    {
        cin >> b;
        binaryzacja(b, filePath, filePath2, filePath3);
        ust(filePath3);
    }

    return 0;
}
