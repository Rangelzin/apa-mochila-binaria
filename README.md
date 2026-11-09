# APA — Mochila Binária (0-1 Knapsack)

Trabalho final de Análise e Projeto de Algoritmos (UFG): implementação e análise de estratégias para a mochila 0-1, em C++17.

## Estrutura

```
apa-mochila/
├── include/
│   └── mochila.hpp              # tipos e contratos comuns a todas as estratégias
├── src/
│   ├── main.cpp                 # executa uma instância ou o benchmark
│   ├── mochila.cpp              # leitura, gerador aleatório, verificador de certificado
│   ├── divisao_conquista.cpp
│   └── programacao_dinamica.cpp
├── data/
│   └── exemplo.txt              # instância de exemplo
├── scripts/
│   └── graficos.py              # CSV -> gráficos
├── results/
│   └── resultados.csv           # saída do benchmark
├── docs/
│   ├── DC_PD_parcial.md         # documentação (vira o PDF de entrega)
│   └── img/                     # gráficos usados no .md
├── build/                       # executáveis (ignorado pelo git)
└── Makefile
```

## Como rodar

### Windows: VS Code (mais fácil)

Requisitos: MinGW-w64 (`g++` no PATH) e, para os gráficos, Python com `pip install matplotlib`.

Abra a pasta no VS Code e use `Ctrl+Shift+P` → **Tasks: Run Task**:

| Task | O que faz |
|---|---|
| `Compilar` (`Ctrl+Shift+B`) | gera `build/mochila.exe` |
| `Rodar exemplo` | resolve `data/exemplo.txt` com todas as estratégias |
| `Benchmark + gráficos` | gera `results/resultados.csv` e os PNGs em `docs/img/` (~20 s) |

### Windows: PowerShell

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/main.cpp src/mochila.cpp src/divisao_conquista.cpp src/programacao_dinamica.cpp -o build/mochila.exe
.\build\mochila.exe data\exemplo.txt

.\build\mochila.exe --benchmark results\resultados.csv
python scripts\graficos.py results\resultados.csv docs\img
```

### Linux / macOS / Git Bash

```bash
make run         # compila e resolve data/exemplo.txt
make benchmark   # CSV + gráficos
```

## Formato da instância

```
n C V
p_1 v_1
...
p_n v_n
```

Exemplo (`data/exemplo.txt`): 4 itens, capacidade 7, alvo 9 → resposta SIM com os itens {2, 3}.

## Adicionando uma estratégia (para o grupo)

1. Declare a função em `include/mochila.hpp`, com a assinatura `Resultado resolverXxx(const Instancia&)`.
2. Implemente-a em `src/xxx.cpp`, incrementando `resultado.operacoes` na operação barômetro.
3. Registre-a em `estrategias()` no `src/main.cpp`. O benchmark passa a medi-la e a comparar o resultado dela com as outras.
4. Acrescente o arquivo às listas `NUCLEO` (Makefile) e `args` (`.vscode/tasks.json`).
As estratégias exponenciais (força bruta e backtracking) não vão aguentar o benchmark atual (n até 1024). Para elas, convém um experimento separado, com n pequeno.

## Gerar o PDF

Requer [pandoc](https://pandoc.org) e uma distribuição LaTeX (MiKTeX no Windows):

```bash
cd docs
pandoc DC_PD_parcial.md -o trabalho.pdf --pdf-engine=xelatex -V geometry:margin=2.5cm -V lang=pt-BR
```
