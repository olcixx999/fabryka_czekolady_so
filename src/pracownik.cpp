#include <iostream>
#include <unistd.h>
#include "common.h"

using namespace std;

bool pobierz_z_kolejki(Kolejka* k) {
    if (k->ilosc_sztuk <= 0) {
        return false;
    }

    int index_bajtowy = k->tail * k->rozmiar_elementu;

    for(int i=0; i < k->rozmiar_elementu; i++) {
        k->dane[index_bajtowy + i] = 0;
    }

    k->tail = (k->tail + 1) % k->max_sztuk;

    k->ilosc_sztuk--;

    return true;
}

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

    cout << "[PRACOWNIK " << typ << "] Zaczynam produkcję.\n";

    while (true) {

        if (!magazyn->magazyn_otwarty) {
            cout << "[PRACOWNIK " << typ << "] Polecenie 2: Magazyn zamkniety. Koniec.\n";
            break;
        }

        if (!magazyn->produkcja_aktywna) {
            cout << "[PRACOWNIK " << typ << "] Polecenie 1. Konczę zmianę.\n";
            break;
        }

        if (!magazyn->fabryka_dziala) {
            cout << "[PRACOWNIK " << typ << "] Polecenie 4: Koniec pracy fabryki.\n";
            break;
        }

        sleep(rand() % 4 + 2); 

        sem_lock(semid);

        bool zrobione = false;

        bool mamA = magazyn->A.ilosc_sztuk > 0;
        bool mamB = magazyn->B.ilosc_sztuk > 0;
        bool mamC = magazyn->C.ilosc_sztuk > 0;
        bool mamD = magazyn->D.ilosc_sztuk > 0;

        if (typ == 1) {
            if (mamA && mamB && mamC) {
                pobierz_z_kolejki(&magazyn->A);
                pobierz_z_kolejki(&magazyn->B);
                pobierz_z_kolejki(&magazyn->C);
                zrobione = true;
            } else {
                 printf("[P1] Stoję bo brak: %s%s%s\n", !mamA ? "A " : "", !mamB ? "B " : "", !mamC ? "C" : "");
            }
        } 
        else if (typ == 2) {
            if (mamA && mamB && mamD) {
                pobierz_z_kolejki(&magazyn->A);
                pobierz_z_kolejki(&magazyn->B);
                pobierz_z_kolejki(&magazyn->D);
                zrobione = true;
            } else {
                 printf("[P2] Stoję bo brak: %s%s%s\n", !mamA ? "A " : "", !mamB ? "B " : "", !mamD ? "D" : "");
            }
        }

        if (zrobione) {
            Raport msg;
            msg.mtype = 1;
            sprintf(msg.tekst, "Pracownik %d wyprodukował czekoladę! (Zapas C: %d, D: %d)", typ, magazyn->C.ilosc_sztuk, magazyn->D.ilosc_sztuk);
            msgsnd(msgid, &msg, sizeof(msg.tekst), IPC_NOWAIT);
        }

        sem_unlock(semid);
    }
    
    shmdt(magazyn);
    return 0;
}