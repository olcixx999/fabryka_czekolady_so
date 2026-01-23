#include "common.h"
#include <sys/wait.h>

using namespace std;

int g_shmid = 0;
int g_semid = 0;
Magazyn* g_magazyn = nullptr;

pid_t pid_kierownik_dostaw = 0;
pid_t pid_pracownik1 = 0;
pid_t pid_pracownik2 = 0;

void sprzatanie_i_wyjscie() {
    if (pid_kierownik_dostaw > 0) kill(pid_kierownik_dostaw, SIGKILL);
    if (pid_pracownik1 > 0) kill(pid_pracownik1, SIGKILL);
    if (pid_pracownik2 > 0) kill(pid_pracownik2, SIGKILL);
    
    while (wait(NULL) > 0);

    if (g_magazyn != nullptr) shmdt(g_magazyn);
    if (g_shmid > 0) shmctl(g_shmid, IPC_RMID, nullptr);
    if (g_semid > 0) semctl(g_semid, 0, IPC_RMID);

    cout << KOLOR_SYS << "[SYSTEM] Zasoby zwolnione. Koniec programu." << KOLOR_RESET << "\n";
    exit(0);
}

void handle_sigint(int sig) {
    (void)sig;
    cout << "\n" << KOLOR_ERR << "[DYREKTOR] Otrzymano SIGINT. Zapis stanu i koniec..." << KOLOR_RESET << "\n";
    
    if (g_magazyn != nullptr) {
        zapisz_stan(g_magazyn);
    }
    
    sprzatanie_i_wyjscie();
}

void handle_sigusr1(int sig) {
    (void)sig;
    if (g_magazyn != nullptr && g_semid > 0) {
        char bufor[512];
        sprintf(bufor, 
            "\n*** [KONTROLA] OTRZYMANO SIGUSR1 ***\n"
            "   Status fabryki: %s\n"
            "   A: %d/%d\n"
            "   B: %d/%d\n"
            "   C: %d/%d\n"
            "   D: %d/%d\n"
            "************************************",
            (g_magazyn->fabryka_dziala ? "AKTYWNA" : "ZATRZYMANA"),
            g_magazyn->A.ilosc_sztuk, g_magazyn->A.max_sztuk,
            g_magazyn->B.ilosc_sztuk, g_magazyn->B.max_sztuk,
            g_magazyn->C.ilosc_sztuk, g_magazyn->C.max_sztuk,
            g_magazyn->D.ilosc_sztuk, g_magazyn->D.max_sztuk
        );
        loguj_komunikat(g_semid, bufor, KOLOR_SYS);
    }
}

void ustaw_semafor(int semid, int sem_num, int wartosc) {
    semctl(semid, sem_num, SETVAL, wartosc);
}

int main() {
    FILE* f = fopen(PLIK_LOGU, "w");
    if(f) { fprintf(f, "--- START SYMULACJI ---\n"); fclose(f); }

    cout << KOLOR_DYR << "[DYREKTOR] Start systemu..." << KOLOR_RESET << endl;

    signal(SIGINT,  handle_sigint);
    signal(SIGUSR1, handle_sigusr1);

    g_shmid = shmget(SHM_KEY, sizeof(Magazyn), IPC_CREAT | 0600);
    sprawdz_blad(g_shmid, "shmget");
    g_magazyn = static_cast<Magazyn*>(shmat(g_shmid, nullptr, 0));
    
    g_semid = semget(SEM_KEY, 10, IPC_CREAT | 0600);
    sprawdz_blad(g_semid, "semget");

    ustaw_semafor(g_semid, SEM_MUTEX, 1);
    ustaw_semafor(g_semid, SEM_LOG, 1); 

    bool wczytano = wczytaj_stan(g_magazyn);

    if (!wczytano) {
        loguj_komunikat(g_semid, "[DYREKTOR] Inicjalizacja pustego magazynu.", KOLOR_DYR);
        inicjalizuj_kolejke(&g_magazyn->A, 1);
        inicjalizuj_kolejke(&g_magazyn->B, 1);
        inicjalizuj_kolejke(&g_magazyn->C, 2);
        inicjalizuj_kolejke(&g_magazyn->D, 3);
    } else {
        loguj_komunikat(g_semid, "[DYREKTOR] Wznowiono pracę z pliku.", KOLOR_DYR);
    }

    ustaw_semafor(g_semid, SEM_EMPTY_A, g_magazyn->A.max_sztuk - g_magazyn->A.ilosc_sztuk);
    ustaw_semafor(g_semid, SEM_FULL_A,  g_magazyn->A.ilosc_sztuk);
    ustaw_semafor(g_semid, SEM_EMPTY_B, g_magazyn->B.max_sztuk - g_magazyn->B.ilosc_sztuk);
    ustaw_semafor(g_semid, SEM_FULL_B,  g_magazyn->B.ilosc_sztuk);
    ustaw_semafor(g_semid, SEM_EMPTY_C, g_magazyn->C.max_sztuk - g_magazyn->C.ilosc_sztuk);
    ustaw_semafor(g_semid, SEM_FULL_C,  g_magazyn->C.ilosc_sztuk);
    ustaw_semafor(g_semid, SEM_EMPTY_D, g_magazyn->D.max_sztuk - g_magazyn->D.ilosc_sztuk);
    ustaw_semafor(g_semid, SEM_FULL_D,  g_magazyn->D.ilosc_sztuk);

    g_magazyn->fabryka_dziala = true;
    g_magazyn->dostawy_aktywne = true;
    g_magazyn->produkcja_aktywna = true;
    g_magazyn->magazyn_otwarty = true;

    pid_kierownik_dostaw = fork();
    if (pid_kierownik_dostaw == 0) { execl("./dostawca", "dostawca", nullptr); exit(1); }
    
    pid_pracownik1 = fork();
    if (pid_pracownik1 == 0) { execl("./pracownik", "pracownik", "1", nullptr); exit(1); }

    pid_pracownik2 = fork();
    if (pid_pracownik2 == 0) { execl("./pracownik", "pracownik", "2", nullptr); exit(1); }

    cout << KOLOR_SYS << "[DYREKTOR] System gotowy (PID: " << getpid() << ")." << KOLOR_RESET << "\n";
    cout << "--- MENU STEROWANIA ---\n";
    cout << "1 - Stop produkcji\n";
    cout << "2 - Zamknij magazyn\n";
    cout << "3 - Stop dostaw\n";
    cout << "4 - ZAPISZ STAN I ZAKONCZ\n";
    cout << "\nPodaj polecenie: ";
    
    int opcja;
    while(cin >> opcja) {
        sem_P(g_semid, SEM_MUTEX);
        
        if (opcja == 1) {
            g_magazyn->produkcja_aktywna = false;
            loguj_komunikat(g_semid, "DYREKTOR: Zatrzymano produkcje (Polecenie 1)", KOLOR_DYR);
            sem_V(g_semid, SEM_MUTEX);
            
            waitpid(pid_pracownik1, NULL, 0);
            waitpid(pid_pracownik2, NULL, 0);
        }
        else if (opcja == 2) {
            g_magazyn->magazyn_otwarty = false;
            loguj_komunikat(g_semid, "DYREKTOR: Zamknieto magazyn (Polecenie 2)", KOLOR_DYR);
            sem_V(g_semid, SEM_MUTEX);
            
            waitpid(pid_kierownik_dostaw, NULL, 0);
            waitpid(pid_pracownik1, NULL, 0);
            waitpid(pid_pracownik2, NULL, 0);
        }
        else if (opcja == 3) {
            g_magazyn->dostawy_aktywne = false;
            loguj_komunikat(g_semid, "DYREKTOR: Zatrzymano dostawy (Polecenie 3)", KOLOR_DYR);
            sem_V(g_semid, SEM_MUTEX);
            
            waitpid(pid_kierownik_dostaw, NULL, 0);
        }
        else if (opcja == 4) {
            loguj_komunikat(g_semid, "[DYREKTOR] Koniec systemu (Polecenie 4)", KOLOR_DYR);
            
            g_magazyn->fabryka_dziala = false;
            g_magazyn->dostawy_aktywne = false;

            sem_V(g_semid, SEM_MUTEX);

            if (pid_kierownik_dostaw > 0) kill(pid_kierownik_dostaw, SIGTERM);
            if (pid_pracownik1 > 0) kill(pid_pracownik1, SIGTERM);
            if (pid_pracownik2 > 0) kill(pid_pracownik2, SIGTERM);

            waitpid(pid_kierownik_dostaw, NULL, 0);
            waitpid(pid_pracownik1, NULL, 0);
            waitpid(pid_pracownik2, NULL, 0);

            zapisz_stan(g_magazyn);
            loguj_komunikat(g_semid, "[SYSTEM] Stan magazynu zapisany do pliku.", KOLOR_SYS);
            
            cout << "Surowiec A: " << g_magazyn->A.ilosc_sztuk << " / " << g_magazyn->A.max_sztuk << " sztuk" << endl;
            cout << "Surowiec B: " << g_magazyn->B.ilosc_sztuk << " / " << g_magazyn->B.max_sztuk << " sztuk" << endl;
            cout << "Surowiec C: " << g_magazyn->C.ilosc_sztuk << " / " << g_magazyn->C.max_sztuk << " sztuk" << endl;
            cout << "Surowiec D: " << g_magazyn->D.ilosc_sztuk << " / " << g_magazyn->D.max_sztuk << " sztuk" << endl;

            sprzatanie_i_wyjscie();
            break;
        }
        else {
             sem_V(g_semid, SEM_MUTEX);
             cout << "Nieznana opcja.\n";
        }
        
        cout << "\nPodaj polecenie: ";
    }
    
    sprzatanie_i_wyjscie();
    return 0;
}