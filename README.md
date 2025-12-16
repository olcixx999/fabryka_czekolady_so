# 🍫 Fabryka Czekolady – Symulacja Produkcji (IPC)

> **Temat projektu:** Temat 3 – Fabryka czekolady  

Projekt jest symulacją działania fabryki czekolady wykorzystującą mechanizmy komunikacji międzyprocesowej (IPC) oraz synchronizacji procesów. System modeluje przepływ składników od dostawców do magazynu, a następnie na linie produkcyjne, przy zachowaniu ograniczeń pojemnościowych i reagowaniu na polecenia sterujące.

---

## 🏭 Opis Systemu

Głównym celem projektu jest symulacja problemu **Producenta i Konsumenta** w środowisku wielu procesów współbieżnych. 

System składa się z:
1.  **Magazynu** o pojemności $N$ jednostek.
2.  **4 Niezależnych Dostawców** dostarczających składniki A, B, C i D.
3.  **2 Stanowisk Produkcyjnych** (Pracowników) pobierających składniki.
4.  **Dyrektora** zarządzającego stanem fabryki.

Stan magazynu jest persystentny – w przypadku nagłego zatrzymania (Polecenie 4), stan jest zapisywany do pliku i odtwarzany przy ponownym uruchomieniu.

---

## ⚙️ Logika Produkcji

### Składniki i Magazynowanie
Każdy składnik zajmuje określoną liczbę jednostek w magazynie:

| Składnik | Rozmiar (jednostki) | Źródło |
| :---: | :---: | :--- |
| **A** | 1 | Dostawca 1 |
| **B** | 1 | Dostawca 2 |
| **C** | 2 | Dostawca 3 |
| **D** | 3 | Dostawca 4 |

### Linie Produkcyjne
Pracownicy pobierają zestawy składników niezbędne do wyprodukowania konkretnego typu czekolady:

* **Stanowisko 1 (Czekolada Typu 1):** Wymaga składników `A + B + C` (Suma: 4 jednostki magazynowe).
* **Stanowisko 2 (Czekolada Typu 2):** Wymaga składników `A + B + D` (Suma: 5 jednostek magazynowych).

---

## 📢 Komendy Dyrektora

Proces `dyrektor` steruje działaniem symulacji poprzez wysyłanie sygnałów/komunikatów do pozostałych procesów.

| Polecenie | Opis Działania |
| :---: | :--- |
| **1** | **Zatrzymanie Fabryki:** Pracownicy kończą produkcję. |
| **2** | **Zatrzymanie Magazynu:** Magazyn kończy pracę. |
| **3** | **Zatrzymanie Dostaw:** Dostawcy przestają dowozić składniki. |
| **4** | **Totalny STOP i Zapis:** Fabryka i magazyn kończą pracę natychmiast. Stan magazynu jest zapisywany do pliku, aby mógł zostać odtworzony po restarcie. |

---

*Projekt realizowany w ramach przedmiotu Systemy Operacyjne.*