#include <iostream>
#include <unistd.h>
#include "common.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Blad: Brak typu pracownika (1 lub 2)" << endl;
        return 1;
    }

    int typ = atoi(argv[1]);

    if (typ != 1 && typ != 2) {
        cerr << "Blad: Nieznany typ pracownika: " << typ << endl;
        return 1;
    }

    cout << "[PRACOWNIK " << typ << "] Gotowy do pracy (PID: " << getpid() << ")\n";
    
    return 0;
}