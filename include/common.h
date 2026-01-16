#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

const int SHM_KEY = 0x1111;
const int SEM_KEY = 0x2222;
const int MSG_KEY = 0x3333;

const int ROZMIAR_SEGMENTU = 100;

struct Kolejka {
    char dane[ROZMIAR_SEGMENTU];
    int head;
    int tail;
    int ilosc_sztuk;
    int rozmiar_elementu;
    int max_sztuk;
};

struct Magazyn {
    Kolejka A;
    Kolejka B;
    Kolejka C;
    Kolejka D;

    bool fabryka_dziala;
    bool dostawy_aktywne;
    bool produkcja_aktywna;
    bool magazyn_otwarty;
};

struct Raport {
    long mtype;
    char tekst[256];
};

inline void sprawdz_blad(int wynik, const char* komunikat){
    if (wynik == -1){
        std::perror(komunikat);
        std::exit(EXIT_FAILURE);
    }
}

inline void inicjalizuj_kolejke(Kolejka* k, int rozmiar_el) {
    k->head = 0;
    k->tail = 0;
    k->ilosc_sztuk = 0;
    k->rozmiar_elementu = rozmiar_el;
    k->max_sztuk = ROZMIAR_SEGMENTU / rozmiar_el;
    std::memset(k->dane, 0, ROZMIAR_SEGMENTU);
}

inline void sem_lock(int semid){
    struct sembuf operacja;
    operacja.sem_num = 0;
    operacja.sem_op = -1;
    operacja.sem_flg = 0;

    if (semop(semid, &operacja, 1) == -1){
        if (errno != EINTR && errno != EIDRM && errno != EINVAL) {
            std::perror("Blad sem_lock");
        }
    }
}

inline void sem_unlock(int semid){
    struct sembuf operacja;
    operacja.sem_num = 0;
    operacja.sem_op = 1;
    operacja.sem_flg = 0;

    if (semop(semid, &operacja, 1) == -1){
        if (errno != EINTR && errno != EIDRM && errno != EINVAL) {
            std::perror("Blad sem_unlock");
        }
    }
}

#endif