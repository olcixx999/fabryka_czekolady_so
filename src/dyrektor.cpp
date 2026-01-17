#include "common.h"
#include <iostream>
#include <sys/wait.h>
#include <thread>

using namespace std;

pid_t pid_kierownik_dostaw = 0;
pid_t pid_pracownik1 = 0;
pid_t pid_pracownik2 = 0;
bool system_dziala = true;

void sprzatanie_i_wyjscie(int shmid, int semid, int msgid, Magazyn* mag) {
    system_dziala = false;
    cout << "\n[DYREKTOR] Sprzątanie fabryki..." << endl;

    sleep(1);

    if (pid_kierownik_dostaw > 0) kill(pid_kierownik_dostaw, SIGTERM);
    if (pid_pracownik1 > 0) kill(pid_pracownik1, SIGTERM);
    if (pid_pracownik2 > 0) kill(pid_pracownik2, SIGTERM);
    
    while (wait(NULL) > 0);
    
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

    if (wczytaj_stan(magazyn)) {
        cout << "[DYREKTOR] Wznowiono pracę. Stan surowców:\n";
        cout << "   A: " << magazyn->A.ilosc_sztuk << " szt.\n";
        cout << "   B: " << magazyn->B.ilosc_sztuk << " szt.\n";
        cout << "   C: " << magazyn->C.ilosc_sztuk << " szt.\n";
        cout << "   D: " << magazyn->D.ilosc_sztuk << " szt.\n";
    } else {
        cout << "[DYREKTOR] Brak pliku zapisu lub błąd. Inicjalizacja pustego magazynu.\n";
        inicjalizuj_kolejke(&magazyn->A, 1);
        inicjalizuj_kolejke(&magazyn->B, 1);
        inicjalizuj_kolejke(&magazyn->C, 2);
        inicjalizuj_kolejke(&magazyn->D, 3);
    }

    magazyn->fabryka_dziala = true;
    magazyn->dostawy_aktywne = true;
    magazyn->produkcja_aktywna = true;
    magazyn->magazyn_otwarty = true;

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
    cout << "1 - Zatrzymaj produkcje\n";
    cout << "2 - Zatrzymaj prace magazynu\n";
    cout << "3 - Zatrzymaj dostawy\n";
    cout << "4 - Koniec pracy calej fabryki\n";
    cout << "------------\n";
    
    int opcja;
    while(cin >> opcja) {
        sem_lock(semid);
        if (opcja == 1) {
            magazyn->produkcja_aktywna = false;
            cout << "\n[DYREKTOR] >>> WYSŁANO POLECENIE 1: STOP PRODUKCJI <<<\n";
        }
        else if (opcja == 2) {
            magazyn->magazyn_otwarty = false;
            cout << "\n[DYREKTOR] >>> POLECENIE 2: ZAMKNIECIE MAGAZYNU <<<\n";
        }
        else if (opcja == 3) {
            magazyn->dostawy_aktywne = false;
            cout << "\n[DYREKTOR] >>> WYSŁANO POLECENIE 3: KONIEC DOSTAW <<<\n";
        }
        else if (opcja == 4) {
            zapisz_stan(magazyn);
            magazyn->fabryka_dziala = false;
            cout << "\n[DYREKTOR] >>> POLECENIE 4: KONIEC PRACY SYSTEMU <<<\n";
            sem_unlock(semid);
            sleep(1);
            break;
        }
        else {
            cout << "Nieznana opcja.\n";
        }
        sem_unlock(semid);
        cout << "\nPodaj polecenie: ";
    }
    
    sprzatanie_i_wyjscie(shmid, semid, msgid, magazyn);
    return 0;
}