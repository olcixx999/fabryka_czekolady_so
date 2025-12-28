#include "common.h"
#include <iostream>
#include <csignal>
#include <sys/wait.h>
#include <thread>

using namespace std;

//zapis i odczyt
//sygnaly do napisania

pid_t pid_kierownik_dostaw = 0;
pid_t pid_pracownik1 = 0;
pid_t pid_pracownik2 = 0;
bool system_dziala = true;

void sprzatanie_i_wyjscie(int shmid, int semid, int msgid, Magazyn* mag) {
    system_dziala = false;
    cout << "\n[DYREKTOR] Sprzątanie fabryki..." << endl;

    if (pid_kierownik_dostaw > 0) kill(pid_kierownik_dostaw, SIGKILL);
    if (pid_pracownik1 > 0) kill(pid_pracownik1, SIGKILL);
    if (pid_pracownik2 > 0) kill(pid_pracownik2, SIGKILL);
    
    shmdt(mag);
    shmctl(shmid, IPC_RMID, nullptr);
    semctl(semid, 0, IPC_RMID);
    msgctl(msgid, IPC_RMID, nullptr);

    cout << "[DYREKTOR] Koniec pracy.\n";
    exit(0);
}

void obsluga_komunikatow(int msgid) {
    Raport msg;
    while(system_dziala) {
        if (msgrcv(msgid, &msg, sizeof(msg.tekst), 0, 0) != -1) {
            cout << "RAPORT: " << msg.tekst << endl;
        }
    }
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
    magazyn->fabryka_dziala = true;
    magazyn->dostawy_aktywne = true;

    pid_kierownik_dostaw = fork();
    if (pid_kierownik_dostaw == 0) {
        execl("./dostawca", "dostawca", nullptr);
        perror("Błąd execl dostawca");
        exit(1);
    }

    cout << "[DYREKTOR] Uruchomiono Kierownika Dostaw (PID: " << pid_kierownik_dostaw << ")" << endl;
    
    pid_pracownik1 = fork();
    if (pid_pracownik1 == 0) {
        execl("./pracownik", "pracownik", "1", nullptr);
        perror("Błąd execl pracownik 1");
        exit(1);
    }
    cout << "[DYREKTOR] Uruchomiono Pracownika 1 (PID: " << pid_pracownik1 << ")" << endl;

    pid_pracownik2 = fork();
    if (pid_pracownik2 == 0) {
        execl("./pracownik", "pracownik", "2", nullptr);
        perror("Błąd execl pracownik 2");
        exit(1);
    }
    cout << "[DYREKTOR] Uruchomiono Pracownika 2 (PID: " << pid_pracownik2 << ")" << endl;
    
    thread t_raporty(obsluga_komunikatow, msgid);
    t_raporty.detach();

    cout << "[DYREKTOR] System działa.\n";
    cout << "--- MENU ---\n";
    cout << "3 - Zatrzymaj dostawy\n";
    cout << "0 - Wyjscie (Ctrl+C)\n";
    cout << "------------\n";
    
    int opcja;
    while(cin >> opcja) {
        if (opcja == 3) {
            sem_lock(semid);
            magazyn->dostawy_aktywne = false;
            sem_unlock(semid);
            cout << "\n[DYREKTOR] >>> WYSŁANO POLECENIE 3: KONIEC DOSTAW <<<\n";
        }
        else if (opcja == 0) {
            break;
        }
        else {
            cout << "Nieznana opcja.\n";
        }
        cout << "\nPodaj polecenie (3 lub 0): ";
    }
    
    sprzatanie_i_wyjscie(shmid, semid, msgid, magazyn);
    return 0;
}