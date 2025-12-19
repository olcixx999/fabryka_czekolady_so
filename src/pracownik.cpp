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

    cout << "[PRACOWNIK " << typ << "] Zaczynam produkcję.\n";

    while (true) {
        sleep(rand() % 4 + 2); 

        sem_lock(semid);

        bool zrobione = false;

        if (typ == 1) {
            if (magazyn->A > 0 && magazyn->B > 0 && magazyn->C > 0) {
                magazyn->A--; 
                magazyn->B--; 
                magazyn->C--;
                magazyn->zajete_miejsce -= 4;
                zrobione = true;
            }
        } 
        else if (typ == 2) {
            if (magazyn->A > 0 && magazyn->B > 0 && magazyn->D > 0) {
                magazyn->A--; 
                magazyn->B--; 
                magazyn->D--;
                magazyn->zajete_miejsce -= 5;
                zrobione = true;
            }
        }

        if (zrobione) {
            Raport msg;
            msg.mtype = 1;
            sprintf(msg.tekst, "Pracownik %d zrobił czekoladę! Stan: %d", typ, magazyn->zajete_miejsce);
            msgsnd(msgid, &msg, sizeof(msg.tekst), IPC_NOWAIT);
        }

        sem_unlock(semid);
    }
    
    shmdt(magazyn);
    return 0;
}