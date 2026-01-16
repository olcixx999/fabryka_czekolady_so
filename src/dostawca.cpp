#include "common.h"
#include <vector>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

using namespace std;

vector<pid_t> dzieci;

bool wstaw_do_kolejki(Kolejka* k, char typ) {
    if (k->ilosc_sztuk >= k->max_sztuk) {
        return false;
    }

    int index_bajtowy = k->head * k->rozmiar_elementu;

    for (int i = 0; i < k->rozmiar_elementu; i++) {
        k->dane[index_bajtowy + i] = typ;
    }

    k->head = (k->head + 1) % k->max_sztuk;
    
    k->ilosc_sztuk++;
    
    return true;
}

void proces_dostawcy(char typ) {
    int shmid = shmget(SHM_KEY, sizeof(Magazyn), 0666);
    if (shmid == -1) sprawdz_blad(-1, "shmget (dostawca)");

    Magazyn* magazyn = static_cast<Magazyn*>(shmat(shmid, nullptr, 0));
    if (magazyn == (void*)-1) sprawdz_blad(-1, "shmat (dostawca)");

    int semid = semget(SEM_KEY, 1, 0666);
    int msgid = msgget(MSG_KEY, 0666);

    srand(time(NULL) ^ getpid());

    printf("[DOSTAWCA %c] Gotowy. Rozmiar elem: %d bajty.\n", typ, (typ=='C'?2:(typ=='D'?3:1)));

    while (true) {

        if (!magazyn->magazyn_otwarty) {
            printf("[DOSTAWCA %c] Polecenie 2: Magazyn zamkniety. Koniec.\n", typ);
            break;
        }

        if (!magazyn->dostawy_aktywne){
            printf("[DOSTAWCA %c] Polecenie 3. Konczę pracę.\n", typ);
            break;
        }

        if (!magazyn->fabryka_dziala) {
            printf("[DOSTAWCA %c] Polecenie 4: Koniec pracy fabryki.\n", typ);
            break;
        }
        
        sleep(rand() % 2 + 1);

        sem_lock(semid);

        Kolejka* moja_kolejka = nullptr;
        switch(typ) {
            case 'A': moja_kolejka = &magazyn->A; break;
            case 'B': moja_kolejka = &magazyn->B; break;
            case 'C': moja_kolejka = &magazyn->C; break;
            case 'D': moja_kolejka = &magazyn->D; break;
        }

        if (wstaw_do_kolejki(moja_kolejka, typ)) {
            Raport msg;
            msg.mtype = 1;
            sprintf(msg.tekst, "[DOSTAWCA %c] Dostarczono. Stan segmentu: %d/%d szt (Head: %d)", 
                    typ, moja_kolejka->ilosc_sztuk, moja_kolejka->max_sztuk, moja_kolejka->head);
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

    while (wait(NULL) > 0);

    printf("[KIEROWNIK DOSTAW] Wszyscy dostawcy zakończyli pracę.\n");
    return 0;
}