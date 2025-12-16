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

    int shmid = shmget(SHM_KEY, sizeof(Magazyn), 0666);
    if (shmid == -1) sprawdz_blad(-1, "shmget");
    
    Magazyn* magazyn = static_cast<Magazyn*>(shmat(shmid, nullptr, 0));
    if (magazyn == (void*)-1) sprawdz_blad(-1, "shmat");

    int semid = semget(SEM_KEY, 1, 0666);
    int msgid = msgget(MSG_KEY, 0666); 

    srand(time(nullptr) ^ getpid());

    for(int i=0; i<3; i++) {
        cout << "[PRACOWNIK " << typ << "] Widzę w magazynie A: " << magazyn->A << "\n";
        sleep(1);
    }
    
    shmdt(magazyn);
    return 0;
}