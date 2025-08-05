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

vector<vector<double>> loadDoubleMatrixFromFile(const string& filePath) {
    vector<vector<double>> matrix;
    ifstream file(filePath);
    string line;

    while (getline(file, line)) {
        istringstream iss(line);
        vector<double> row;
        double value;
        while (iss >> value) {
            row.push_back(value);
            if (iss.peek() == ',') // Jeśli używasz przecinków jako separatorów
                iss.ignore();
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


vector<vector<int>> dilation(int neighborhood, string filePath) {
    vector<vector<int>> matrix = loadMatrixFromFile(filePath);
    int rows = matrix.size();        // Liczba wierszy
    int cols = matrix[0].size();

    vector<vector<int>> output(rows, vector<int>(cols, 255)); // Domyślnie wszystkie białe (255)

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Jeśli piksel jest czarny, oznacz wszystkie sąsiadujące na czarno
            if (matrix[i][j] == 0) {
                for (int di = -neighborhood / 2; di <= neighborhood / 2; di++) 
                {
                    for (int dj = -neighborhood / 2; dj <= neighborhood / 2; dj++)
                    {
                        int ni = i + di;
                        int nj = j + dj;

                        // Sprawdź, czy sąsiad jest w granicach
                        if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                            output[ni][nj] = 0; // Ustaw na czarno, jeśli którykolwiek sąsiad jest czarny
                        }
                    }
                }
            }
        }
    }

    return output;
}


vector<vector<int>> erode(int neighborhood, string filePath) {
    vector<vector<int>> matrix = loadMatrixFromFile(filePath);
    int rows = matrix.size();        // Liczba wierszy
    int cols = matrix[0].size();

    vector<vector<int>> output(rows, vector<int>(cols, 0));
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (matrix[i][j] != 0)
            {
                for (int di = -neighborhood / 2; di <= neighborhood / 2; di++) 
                {
                    for (int dj = -neighborhood / 2; dj <= neighborhood / 2; dj++) 
                    {
                        int ni = i + di;
                        int nj = j + dj;
                        if (ni >= 0 && ni < rows && nj >= 0 && nj < cols)
                            output[ni][nj] = 255;
                    }
                }
            }
        }
    }

    return output;
}

vector<vector<int>> convolution(string filePath, string filePath2)
{
    vector<vector<int>> matrix = loadMatrixFromFile(filePath);
    int rows = matrix.size();        // Liczba wierszy
    int cols = matrix[0].size();

    vector<vector<double>> weight = loadDoubleMatrixFromFile(filePath2);
    int rows_w = weight.size();        // Liczba wierszy
    int cols_w = weight[0].size();

    vector<vector<int>> output(rows, vector<int>(cols, 0));

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            double new_value = 0.0;
            for (int wi = 0; wi < rows_w; wi++)
            {
                for (int wj = 0; wj < cols_w; wj++)
                {
                    int r = i + wi - rows_w/2;
                    int c = j + wj - cols_w/2;
                    if (r >= 0 && r < rows && c >= 0 && c < cols)
                    {
                        new_value += matrix[r][c] * weight[wi][wj];
                    }
                }
            }
            if (new_value > 255)
                new_value = 255;
            else if (new_value < 0)
                new_value = 0;
            output[i][j] = static_cast<int>(round(new_value));
            //std::cout << "New value for pixel (" << i << ", " << j << "): " << new_value << std::endl;
        }
    }

    return output;
}


int main()
{
    //0 - czarny
    //255 - biały

    //operacje morfologiczne
   // vector<vector<int>> output = erode(3, "zz_matrix.txt");
   // saveImageToFile(output, "output.bmp");
   //// ust("output.bmp");
   // saveMatrixToFile("output.txt", output);
   // loadMatrixFromFile("output.txt");

   // vector<vector<int>> outpute = dilation(10, "output.txt");
   // saveImageToFile(outpute, "outputt.bmp");
   // ust("outputt.bmp");

    /*vector<vector<int>> outpute = erode(3, "output.txt");
    saveImageToFile(outpute, "outputt.bmp");
    ust("outputt.bmp");*/

    ////zad 2
    /*output = dilation(3, "zz_matrix.txt");
    saveImageToFile(output, "output.bmp");
    ust("output.bmp"); */

    //maska
    /*string filePath = "po_matrix.txt";
    string filePath2 = "Gauss.txt";
    vector<vector<int>> output = convolution(filePath, filePath2);
    saveImageToFile(output, "zad3.bmp");
    ust("zad3.bmp");*/

    //maska pc
    string filePath = "Mapa_MD_no_terrain_low_res_Gray.txt";
    string filePath2 = "dp.txt";
    vector<vector<int>> loadMatrixFromFile(string filePath);
    vector<vector<double>> loadDoubleMatrixFromFile(string filePath2);
    vector<vector<int>> output = convolution(filePath, filePath2);
    saveImageToFile(output, "zad3.bmp");
    ust("zad3.bmp");

   

    //saveMatrixToFile("output.txt", output);
    //loadMatrixFromFile("output.txt");

    //vector<vector<int>> outpute = erode(3, "output.txt");
    //saveImageToFile(outpute, "outputt.bmp");
    //ust("outputt.bmp");
    

    return 0;
}
