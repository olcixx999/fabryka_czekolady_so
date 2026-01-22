#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fstream>

const int SHM_KEY = 0x1111;
const int SEM_KEY = 0x2222;
const char* PLIK_STANU = "stan_magazynu.bin";
const char* PLIK_LOGU  = "raport_symulacji.txt";

const int ROZMIAR_SEGMENTU = 100;

const char* KOLOR_RESET   = "\033[0m";
const char* KOLOR_DYR     = "\033[1;36m";
const char* KOLOR_DOST    = "\033[1;32m";
const char* KOLOR_PRAC    = "\033[1;33m";
const char* KOLOR_SYS     = "\033[1;35m";
const char* KOLOR_ERR     = "\033[1;31m";

const int SEM_MUTEX = 0;   
const int SEM_EMPTY_A = 1;
const int SEM_FULL_A  = 2;
const int SEM_EMPTY_B = 3;
const int SEM_FULL_B  = 4;
const int SEM_EMPTY_C = 5;
const int SEM_FULL_C  = 6;
const int SEM_EMPTY_D = 7;
const int SEM_FULL_D  = 8;
const int SEM_LOG     = 9; 

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

inline void sprawdz_blad(int wynik, const char* komunikat){
    if (wynik == -1){
        std::cerr << KOLOR_ERR;
        std::perror(komunikat);
        std::cerr << KOLOR_RESET;
        std::exit(EXIT_FAILURE);
    }
}

inline void zapisz_stan(Magazyn* m) {
    std::ofstream plik(PLIK_STANU, std::ios::binary);
    if (plik.is_open()) {
        plik.write((char*)m, sizeof(Magazyn));
        plik.close();
    }
}

inline bool wczytaj_stan(Magazyn* m) {
    std::ifstream plik(PLIK_STANU, std::ios::binary);
    if (plik.is_open()) {
        plik.read((char*)m, sizeof(Magazyn));
        plik.close();
        return true;
    }
    return false;
}

inline void inicjalizuj_kolejke(Kolejka* k, int rozmiar_el) {
    k->head = 0;
    k->tail = 0;
    k->ilosc_sztuk = 0;
    k->rozmiar_elementu = rozmiar_el;
    k->max_sztuk = ROZMIAR_SEGMENTU / rozmiar_el;
    std::memset(k->dane, 0, ROZMIAR_SEGMENTU);
}

inline void sem_P(int semid, int sem_num){
    struct sembuf operacja;
    operacja.sem_num = sem_num;
    operacja.sem_op = -1;
    operacja.sem_flg = 0;

    while (semop(semid, &operacja, 1) == -1) {
        if (errno == EINTR) continue;
        if (errno != EIDRM && errno != EINVAL) std::perror("Blad sem_P");
        break;
    }
}

inline void sem_V(int semid, int sem_num){
    struct sembuf operacja;
    operacja.sem_num = sem_num;
    operacja.sem_op = 1;
    operacja.sem_flg = 0;

    while (semop(semid, &operacja, 1) == -1) {
        if (errno == EINTR) continue;
        if (errno != EIDRM && errno != EINVAL) std::perror("Blad sem_V");
        break;
    }
}

inline void loguj_komunikat(int semid, const char* komunikat, const char* kolor = KOLOR_SYS) {
    sem_P(semid, SEM_LOG); 

    std::cout << kolor << komunikat << KOLOR_RESET << std::endl;

    FILE* plik = std::fopen(PLIK_LOGU, "a");
    if (plik != nullptr) {
        std::fprintf(plik, "%s\n", komunikat);
        std::fclose(plik);
    }

    sem_V(semid, SEM_LOG); 
}

#endif