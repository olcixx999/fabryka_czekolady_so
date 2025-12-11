#include "common.h"
#include <vector>
#include <signal.h>
#include <sys/wait.h>

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
        sleep(rand() % 3 + 1);

        sem_lock(semid);

        if (magazyn->zajete_miejsce + rozmiar <= magazyn->pojemnosc_max) {
            
            switch(typ) {
                case 'A': magazyn->A++; break;
                case 'B': magazyn->B++; break;
                case 'C': magazyn->C++; break;
                case 'D': magazyn->D++; break;
            }
            magazyn->zajete_miejsce += rozmiar;

            Raport msg;
            msg.mtype = 1;
            sprintf(msg.tekst, "Dostawca %c dostarczył towar. Stan magazynu: %d/%d", 
                         typ, magazyn->zajete_miejsce, magazyn->pojemnosc_max);

            msgsnd(msgid, &msg, sizeof(msg.tekst), 0);
            
            printf("[DOSTAWCA %c] Dostarczono. Stan: %d\n", typ, magazyn->zajete_miejsce);
            
        } else {
            printf("Magazyn pełny");
        }

        sem_unlock(semid);
    }
}

void handle_stop_signal(int sig) {
    (void)sig;
    printf("[KIEROWNIK DOSTAW] Otrzymano sygnał STOP. Zwalniam dostawców...\n");
    
    for (pid_t pid : dzieci) {
        kill(pid, SIGKILL);
    }
    
    while (wait(NULL) > 0);
    
    exit(0);
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
        pause();
    }

    return 0;
}