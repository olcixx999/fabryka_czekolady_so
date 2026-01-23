CXX = g++

CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

SRC_DIR = src

TARGETS = dyrektor dostawca pracownik pokaz_stan

all: $(TARGETS)

dyrektor: $(SRC_DIR)/dyrektor.cpp include/common.h
	$(CXX) $(CXXFLAGS) -o dyrektor $(SRC_DIR)/dyrektor.cpp

dostawca: $(SRC_DIR)/dostawca.cpp include/common.h
	$(CXX) $(CXXFLAGS) -o dostawca $(SRC_DIR)/dostawca.cpp

pracownik: $(SRC_DIR)/pracownik.cpp include/common.h
	$(CXX) $(CXXFLAGS) -o pracownik $(SRC_DIR)/pracownik.cpp

pokaz_stan: $(SRC_DIR)/pokaz_stan.cpp include/common.h
	$(CXX) $(CXXFLAGS) -o pokaz_stan $(SRC_DIR)/pokaz_stan.cpp

clean:
	rm -f $(TARGETS) *.o raport_symulacji.txt stan_magazynu.bin
	rm -rf *.dSYM
	