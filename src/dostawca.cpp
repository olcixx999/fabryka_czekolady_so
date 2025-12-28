#include "common.h"
#include <vector>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

using namespace std;

vector<pid_t> dzieci;

void proces_dostawcy(char typ) {
    int shmid = shmget(SHM_KEY, sizeof(Magazyn), 0666);
    if (shmid == -1) sprawdz_blad(-1, "shmget (dostawca)");

    Magazyn* magazyn = static_cast<Magazyn*>(shmat(shmid, nullptr, 0));
    if (magazyn == (void*)-1) sprawdz_blad(-1, "shmat (dostawca)");

    int semid = semget(SEM_KEY, 1, 0666);
    int msgid = msgget(MSG_KEY, 0666);

    srand(time(NULL) ^ getpid());

    int rozmiar = 1;
    if (typ == 'C') rozmiar = 2;
    if (typ == 'D') rozmiar = 3;

    printf("[DOSTAWCA %c] Gotowy do pracy (PID: %d, Rozmiar: %d)\n", typ, getpid(), rozmiar);

    while (true) {
        if (!magazyn->fabryka_dziala) break;

        if (!magazyn->dostawy_aktywne){
            printf("[DOSTAWCA %c] Otrzymano Polecenie 3. Konczę pracę.\n", typ);
            break;
        }
        
        sleep(rand() % 2 + 1);

        sem_lock(semid);

        int pojemnosc = magazyn->pojemnosc_max;
        int max_sztuk = 0;
        int obecna_ilosc = 0;

        switch(typ) {
            case 'A': 
                max_sztuk = (int)(pojemnosc * 0.30); 
                obecna_ilosc = magazyn->A; 
                break;
            case 'B': 
                max_sztuk = (int)(pojemnosc * 0.30); 
                obecna_ilosc = magazyn->B; 
                break;
            case 'C': 
                max_sztuk = (int)((pojemnosc * 0.20) / 2); 
                obecna_ilosc = magazyn->C; 
                break;
            case 'D': 
                max_sztuk = (int)((pojemnosc * 0.15) / 3); 
                obecna_ilosc = magazyn->D; 
                break;
        }

        if (max_sztuk < 1) max_sztuk = 1;

        bool jest_miejsce = (magazyn->zajete_miejsce + rozmiar <= magazyn->pojemnosc_max);
        
        bool limit_ok = (obecna_ilosc < max_sztuk);

        if (jest_miejsce && limit_ok) {
            
            switch(typ) {
                case 'A': magazyn->A++; break;
                case 'B': magazyn->B++; break;
                case 'C': magazyn->C++; break;
                case 'D': magazyn->D++; break;
            }

            magazyn->zajete_miejsce += rozmiar;

            Raport msg;
            msg.mtype = 1;
            sprintf(msg.tekst, "[DOSTAWDA %c] Dostarczono towar. Stan magazynu: %d/%d", 
                         typ, magazyn->zajete_miejsce, magazyn->pojemnosc_max);

            msgsnd(msgid, &msg, sizeof(msg.tekst), 0);
        } else {

        }

        sem_unlock(semid);
    }
}

void handle_stop_signal(int sig) {
    (void)sig;
}

int main() {
    signal(SIGUSR1, handle_stop_signal);

    char typy[] = {'A', 'B', 'C', 'D'};

    printf("[KIEROWNIK DOSTAW] Uruchamiam 4 dostawców...\n");

    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            signal(SIGUSR1, SIG_DFL); 
            proces_dostawcy(typy[i]);
            exit(0);
        } else if (pid > 0) {
            dzieci.push_back(pid);
        } else {
            perror("Błąd fork");
        }
    }

    while (true) {
        int status;
        if (wait(&status) == -1) break;
    }

    printf("[KIEROWNIK DOSTAW] Wszyscy dostawcy zakończyli pracę.\n");
    return 0;
}