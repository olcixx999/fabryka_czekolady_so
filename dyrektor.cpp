#include "common.h"
#include <iostream>

using namespace std;

int main() {
    cout << "[DYREKTOR] Uruchamia system fabryki" << endl;
    
    int shmid = shmget(SHM_KEY, sizeof(Magazyn), IPC_CREAT | 0666);
    sprawdz_blad(shmid, "shmget - tworzenie pamieci");

    Magazyn* magazyn = static_cast<Magazyn*>(shmat(shmid, nullptr, 0));
    if (magazyn == (void*)-1) sprawdz_blad(-1, "shmat - dolaczanie");

    magazyn->A = 0;
    magazyn->B = 0;
    magazyn->C = 0;
    magazyn->D = 0;
    magazyn->zajete_miejsce = 0;
    magazyn->pojemnosc_max = POJEMNOSC_MAGAZYNU;
    magazyn->fabryka_dziala = true;

    cout << "[DYREKTOR] Pamiec utworzona i zainicjalizowana" << endl;

    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    sprawdz_blad(semid, "semget - tworzenie semafora");

    if (semctl(semid, 0, SETVAL, 1) == -1) sprawdz_blad(-1, "semctl - inicjalizacja");

    cout << "[DYREKTOR] Semafor utworzony i ustawiony na 1." << endl;

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    sprawdz_blad(msgid, "msgget - tworzenie kolejki");

    cout << "\nZasoby gotowe. Naciśnij ENTER, aby posprzątać i zakończyć...";
    cin.get();

    cout << "[DYREKTOR] Usuwanie zasobów..." << endl;

    shmdt(magazyn);
    shmctl(shmid, IPC_RMID, nullptr);
    semctl(semid, 0, IPC_RMID);
    msgctl(msgid, IPC_RMID, nullptr);

    cout << "[DYREKTOR] Koniec." << endl;
    return 0;
}