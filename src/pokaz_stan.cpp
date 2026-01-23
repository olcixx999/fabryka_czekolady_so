#include <fstream>
#include "common.h"

using namespace std;

int main() {
    Magazyn stan;

    ifstream plik(PLIK_STANU, std::ios::binary);

    if (!plik) {
        cerr << KOLOR_ERR << "Błąd: Nie znaleziono pliku '" << PLIK_STANU << "'!" << KOLOR_RESET << endl;
        cerr << "Uruchom najpierw symulację, aby utworzyć plik." << endl;
        return 1;
    }

    plik.read((char*)&stan, sizeof(Magazyn));
    
    if (!plik) {
        cerr << KOLOR_ERR << "Błąd: Plik jest uszkodzony lub pusty." << KOLOR_RESET << endl;
        return 1;
    }
    plik.close();

    cout << "========================================" << endl;
    cout << "      ODCZYT STANU MAGAZYNU Z PLIKU     " << endl;
    cout << "========================================" << endl;
    
    cout << "Surowiec A: " << stan.A.ilosc_sztuk << " / " << stan.A.max_sztuk << " sztuk" << endl;
    cout << "Surowiec B: " << stan.B.ilosc_sztuk << " / " << stan.B.max_sztuk << " sztuk" << endl;
    cout << "Surowiec C: " << stan.C.ilosc_sztuk << " / " << stan.C.max_sztuk << " sztuk" << endl;
    cout << "Surowiec D: " << stan.D.ilosc_sztuk << " / " << stan.D.max_sztuk << " sztuk" << endl;

    return 0;
}