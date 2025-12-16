#include "common.h"
#include <iostream>
#include <csignal>
#include <sys/wait.h>

using namespace std;

//zapis i odczyt
//sygnaly do napisania

pid_t pid_kierownik_dostaw = 0;

void sprzatanie_i_wyjscie(int shmid, int semid, int msgid, Magazyn* mag) {
    cout << "\n[DYREKTOR] Sprzątanie fabryki..." << endl;

    if (pid_kierownik_dostaw > 0) kill(pid_kierownik_dostaw, SIGKILL);
    
    shmdt(mag);
    shmctl(shmid, IPC_RMID, nullptr);
    semctl(semid, 0, IPC_RMID);
    msgctl(msgid, IPC_RMID, nullptr);

    exit(0);
}

int main() {
    cout << "[DYREKTOR] Start systemu..." << endl;

    int shmid = shmget(SHM_KEY, sizeof(Magazyn), IPC_CREAT | 0666);
    sprawdz_blad(shmid, "shmget");
    Magazyn* magazyn = static_cast<Magazyn*>(shmat(shmid, nullptr, 0));
    
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    sprawdz_blad(semid, "semget");
    semctl(semid, 0, SETVAL, 1);

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    sprawdz_blad(msgid, "msgget");

    magazyn->A = 0; magazyn->B = 0; magazyn->C = 0; magazyn->D = 0;
    magazyn->zajete_miejsce = 0;
    magazyn->pojemnosc_max = POJEMNOSC_MAGAZYNU;

    pid_kierownik_dostaw = fork();
    if (pid_kierownik_dostaw == 0) {
        execl("./dostawca", "dostawca", nullptr);
        perror("Błąd execl dostawca");
        exit(1);
    }

    cout << "[DYREKTOR] Uruchomiono Kierownika Dostaw (PID: " << pid_kierownik_dostaw << ")" << endl;
    
    cout << "[DYREKTOR] System działa. Nasłuchuję komunikatów (Ctrl+C aby przerwać)...\n";
    
    Raport msg;
    while(true) {
        if (msgrcv(msgid, &msg, sizeof(msg.tekst), 0, 0) != -1) {
            cout << "RAPORT: " << msg.tekst << endl;
        }
    }
    
    return 0;
}