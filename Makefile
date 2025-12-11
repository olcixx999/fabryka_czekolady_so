CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread

TARGETS = dyrektor dostawca pracownik

all: $(TARGETS)

dyrektor: dyrektor.cpp common.h
	$(CXX) $(CXXFLAGS) -o dyrektor dyrektor.cpp

dostawca: dostawca.cpp common.h
	$(CXX) $(CXXFLAGS) -o dostawca dostawca.cpp

pracownik: pracownik.cpp common.h
	$(CXX) $(CXXFLAGS) -o pracownik pracownik.cpp

clean:
	rm -f $(TARGETS) *.o raport.txt stan_magazynu.dat
	rm -rf *.dSYM

tar:
	tar -cvzf projekt_systemy.tar.gz *.cpp *.h Makefile