#include "common.h"

using namespace std;

void pobierz_z_kolejki(Kolejka* k) {
    int index_bajtowy = k->tail * k->rozmiar_elementu;
    for(int i=0; i < k->rozmiar_elementu; i++) {
        k->dane[index_bajtowy + i] = 0;
    }
    k->tail = (k->tail + 1) % k->max_sztuk;
    k->ilosc_sztuk--;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    int typ = atoi(argv[1]);

    //srand(time(NULL) ^ getpid());

    int shmid = shmget(SHM_KEY, sizeof(Magazyn), 0666);
    if (shmid == -1) sprawdz_blad(-1, "shmget");
    Magazyn* magazyn = static_cast<Magazyn*>(shmat(shmid, nullptr, 0));
    
    int semid = semget(SEM_KEY, 10, 0666);

    char bufor[256];

    while (true) {
        if (!magazyn->fabryka_dziala || !magazyn->produkcja_aktywna) break;

        if (typ == 1) {
            sem_P(semid, SEM_FULL_A);
            sem_P(semid, SEM_FULL_B);
            sem_P(semid, SEM_FULL_C);
        } else {
            sem_P(semid, SEM_FULL_A);
            sem_P(semid, SEM_FULL_B);
            sem_P(semid, SEM_FULL_D);
        }

        if (!magazyn->fabryka_dziala || !magazyn->magazyn_otwarty) break;

        sem_P(semid, SEM_MUTEX);
        
        pobierz_z_kolejki(&magazyn->A);
        pobierz_z_kolejki(&magazyn->B);
        if (typ == 1) pobierz_z_kolejki(&magazyn->C);
        else          pobierz_z_kolejki(&magazyn->D);
        
        sem_V(semid, SEM_MUTEX);

        sem_V(semid, SEM_EMPTY_A);
        sem_V(semid, SEM_EMPTY_B);
        if (typ == 1) sem_V(semid, SEM_EMPTY_C);
        else          sem_V(semid, SEM_EMPTY_D);

        sprintf(bufor, "[PRACOWNIK %d] Wyprodukowano czekolade!", typ);

        loguj_komunikat(semid, bufor, KOLOR_PRAC);
        
        //sleep((rand() % 3) + 1);
    }
    
    shmdt(magazyn);
    return 0;
}