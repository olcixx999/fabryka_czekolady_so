#include "common.h"
#include <vector>
#include <signal.h>
#include <sys/wait.h>

using namespace std;

vector<pid_t> dzieci;

void wstaw_do_kolejki(Kolejka* k, char typ) {
    int index_bajtowy = k->head * k->rozmiar_elementu;
    for (int i = 0; i < k->rozmiar_elementu; i++) {
        k->dane[index_bajtowy + i] = typ;
    }
    k->head = (k->head + 1) % k->max_sztuk;
    k->ilosc_sztuk++;
}

void proces_dostawcy(char typ) {
    //srand(time(NULL) ^ getpid());

    int shmid = shmget(SHM_KEY, sizeof(Magazyn), 0666);
    if (shmid == -1) sprawdz_blad(-1, "shmget");
    Magazyn* magazyn = static_cast<Magazyn*>(shmat(shmid, nullptr, 0));

    int semid = semget(SEM_KEY, 10, 0666);

    int sem_empty = 0, sem_full = 0;
    Kolejka* moja_kolejka = nullptr;

    switch(typ) {
        case 'A': moja_kolejka = &magazyn->A; sem_empty = SEM_EMPTY_A; sem_full = SEM_FULL_A; break;
        case 'B': moja_kolejka = &magazyn->B; sem_empty = SEM_EMPTY_B; sem_full = SEM_FULL_B; break;
        case 'C': moja_kolejka = &magazyn->C; sem_empty = SEM_EMPTY_C; sem_full = SEM_FULL_C; break;
        case 'D': moja_kolejka = &magazyn->D; sem_empty = SEM_EMPTY_D; sem_full = SEM_FULL_D; break;
    }

    char bufor[256];

    while (true) {
        if (!magazyn->fabryka_dziala || !magazyn->dostawy_aktywne) break;

        sem_P(semid, sem_empty); 
        
        if (!magazyn->fabryka_dziala || !magazyn->magazyn_otwarty) break;

        sem_P(semid, SEM_MUTEX); 
        wstaw_do_kolejki(moja_kolejka, typ);
        int stan = moja_kolejka->ilosc_sztuk;
        int max_s = moja_kolejka->max_sztuk;
        sem_V(semid, SEM_MUTEX); 

        sem_V(semid, sem_full); 

        sprintf(bufor, "[DOSTAWCA %c] Dostarczono. Stan: %d/%d", typ, stan, max_s);

        loguj_komunikat(semid, bufor, KOLOR_DOST);
        
        //usleep((rand() % 400000) + 100000); 
    }
    exit(0);
}

void handle_stop_signal(int sig) { (void)sig; exit(0); }

int main() {
    signal(SIGUSR1, handle_stop_signal);
    char typy[] = {'A', 'B', 'C', 'D'};
    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid == 0) { proces_dostawcy(typy[i]); exit(0); }
        else dzieci.push_back(pid);
    }
    while (wait(NULL) > 0);
    return 0;
}