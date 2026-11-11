# Linux, macOS ou Git Bash. No Windows (PowerShell), veja o README ou use as tasks do VS Code.
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
CPPFLAGS += -Iinclude
PYTHON   ?= python3

NUCLEO = src/mochila.cpp src/forca_bruta.cpp src/backtracking.cpp src/divisao_conquista.cpp src/programacao_dinamica.cpp src/guloso.cpp src/aproximado.cpp
HEADER = include/mochila.hpp

all: build/mochila

build/mochila: src/main.cpp $(NUCLEO) $(HEADER)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp $(NUCLEO) -o $@

run: build/mochila
	./build/mochila data/exemplo.txt

benchmark: build/mochila
	./build/mochila --benchmark results/resultados.csv
	./build/mochila --qualidade results/qualidade.csv
	./build/mochila --exponencial results/exponencial.csv
	$(PYTHON) scripts/graficos.py results/resultados.csv docs/img results/qualidade.csv results/exponencial.csv

clean:
	rm -f build/mochila build/*.exe

.PHONY: all run benchmark clean