# Linux, macOS ou Git Bash. No Windows (PowerShell), veja o README ou use as tasks do VS Code.
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
CPPFLAGS += -Iinclude
SAN      ?= -fsanitize=address,undefined   # use SAN= para desativar (ex.: MinGW não suporta)
PYTHON   ?= python3

NUCLEO = src/mochila.cpp src/divisao_conquista.cpp src/programacao_dinamica.cpp
HEADER = include/mochila.hpp

all: build/mochila build/testes

build/mochila: src/main.cpp $(NUCLEO) $(HEADER)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp $(NUCLEO) -o $@

build/testes: tests/testes.cpp $(NUCLEO) $(HEADER)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SAN) tests/testes.cpp $(NUCLEO) -o $@

run: build/mochila
	./build/mochila data/exemplo.txt

test: build/testes
	./build/testes

benchmark: build/mochila
	./build/mochila --benchmark results/resultados.csv
	$(PYTHON) scripts/graficos.py results/resultados.csv docs/img

clean:
	rm -f build/mochila build/testes build/*.exe

.PHONY: all run test benchmark clean