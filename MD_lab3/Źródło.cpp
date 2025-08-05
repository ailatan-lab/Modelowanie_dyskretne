#include <iostream>
#include <vector>
#include <ctime>
#include <stdexcept>
#include <fstream>
#include <iomanip>

using namespace std;

vector<int> ruleToBinary(int rule) 
{
    if (rule < 0 || rule > 255) 
        throw out_of_range("Regula musi byc w zakresie 0-255.");

    vector<int> binaryRule(8);
    for (int i = 0; i < 8; ++i)
    {
        binaryRule[7 - i] = (rule >> i) & 1;
    }
    return binaryRule;
}

int applyRule(const vector<int>& binaryRule, int left, int center, int right)
{
    int index = (left << 2) | (center << 1) | right;
    return binaryRule[index];
}

void updateCells(vector<int>& cells, const vector<int>& binaryRule) 
{
    vector<int> newCells(cells.size());

    // Zachowanie pierwszej i ostatniej komórki
    newCells[0] = cells[0];
    newCells[cells.size() - 1] = cells[cells.size() - 1]; 

    for (size_t i = 1; i < cells.size() - 1; ++i)
    {
        newCells[i] = applyRule(binaryRule, cells[i - 1], cells[i], cells[i + 1]);
    }

    cells = newCells; 
}

void updateCellsPeriodic(vector<int>& cells, const vector<int>& binaryRule)
{
    vector<int> newCells(cells.size());
    for (size_t i = 0; i < cells.size(); ++i) 
    { 
        int left = cells[(i - 1 + cells.size()) % cells.size()]; 
        int center = cells[i];
        int right = cells[(i + 1) % cells.size()];
        newCells[i] = applyRule(binaryRule, left, center, right);
    }
    cells = newCells;
}

void updateCellsAbsorptive(vector<int>& cells, const vector<int>& binaryRule)
{
    vector<int> newCells(cells.size());
    for (size_t i = 0; i < cells.size(); ++i) 
    {
        int left = (i == 0) ? 0 : cells[i - 1]; // Lewy s¹siad, 0 dla pierwszej komórki
        int center = cells[i]; // Obecna komórka
        int right = (i == cells.size() - 1) ? 0 : cells[i + 1]; // Prawy s¹siad, 0 dla ostatniej komórki

        newCells[i] = applyRule(binaryRule, left, center, right); 
    }
    cells = newCells; 
}

// Funkcja do wyœwietlania stanu komórek
void displayCells(const vector<int>& cells)
{
    for (int cell : cells) 
    {
        cout << (cell ? '#' : '.');
    }
    cout << endl;
}

void saveToTXT(const vector<int>& cells, int step, const string& filename, int rule) 
{
    ofstream file;
    file.open(filename, ios_base::app);

    if (file.is_open()) 
    {
        if (rule >= 100)
        {
            if(step <= 9)
                file << "      Step " << step << ",  Rule " << rule << ": "; 
            else
                file << "      Step " << step << ", Rule " << rule << ": ";
        }
        else
        {
            if(step <= 9)
                file << "      Step " << step << ",  Rule  " << rule << ": "; 
            else
                file << "      Step " << step << ", Rule  " << rule << ": "; 
        }

        
        for (size_t i = 0; i < cells.size(); ++i)
        {
            file << setw(2) << (cells[i] ? '#' : '.'); 
            if (i < cells.size() - 1) 
                file << ' '; 
        }
        file << endl; 
        file.close();
    }
    else 
        cerr << "B³¹d otwarcia pliku: " << filename << endl;
}

void cleanTXT(string filename)
{
    ofstream file;
    file.open(filename, ios::out | ios::trunc);
}

void runSimulation(int stepsPerRule, int cellCount, const vector<int>& rules) 
{
    vector<int> cells(cellCount);
    int ruleIndex = 0; 

    ofstream file("simulation_output.txt");
    if (file.is_open()) 
    {
        srand(static_cast<unsigned int>(time(0)));
        file << "Poczatkowy stan komorek: ";
        for (int i = 0; i < cellCount; ++i)
        {
            cells[i] = rand() % 2; 
            file << setw(2) << (cells[i] ? '#' : '.'); 
            if (i < cellCount - 1) 
                file << ' '; 
        }
        file << endl;
    }
    cout << endl << "Poczatkowy stan komorek" << endl;
    displayCells(cells);


    for (int ruleStep = 0; ruleStep < rules.size(); ++ruleStep) 
    {
        int currentRule = rules[ruleStep];
        vector<int> binaryRule = ruleToBinary(currentRule); 
        cout << endl << "Wykonywanie reguly: " << currentRule << endl;
        for (int step = 0; step < stepsPerRule; ++step)
        {
            displayCells(cells);
            saveToTXT(cells, step, "simulation_output.txt", currentRule);
            //if (step == stepsPerRule - 1)
              //  saveToTXT(cells, step, "simulation_output.txt", currentRule);
            updateCells(cells, binaryRule);     //WARUNKI BRZEGOWE
        }
    }
}

int main() {
    int steps = 21; 
    int cellCount = 31;    

    cout << "Podaj liczbe komorek: ";
    cin >> cellCount;
    cout << "Podaj liczbe iteracji: ";
    cin >> steps;

    if (cellCount <= 0 || steps <= 0) 
    {
        cerr << "Liczba komorek i krokow musi byc wieksza od 0" << endl;
        return 1; 
    }

    vector<int> rules = { 40, 63, 26, 190 }; 
    cleanTXT("simulation_output.txt");

    try 
    {
        runSimulation(steps, cellCount, rules);
    }
    catch (const exception& e) 
    {
        cerr << "Wystapil blad: " << e.what() << endl; 
        return 1; 
    }

    return 0;
}
