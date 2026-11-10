# APA — Mochila Binária (0-1 Knapsack)

Trabalho final de Análise e Projeto de Algoritmos (UFG): implementação e análise de estratégias para a mochila 0-1, em C++17.

## Estrutura

```
apa-mochila/
├── include/
│   └── mochila.hpp              # tipos e contratos comuns a todas as estratégias
├── src/
│   ├── main.cpp                 # executa uma instância, o benchmark ou o experimento de qualidade
│   ├── mochila.cpp              # leitura, gerador aleatório, verificador de certificado
│   ├── divisao_conquista.cpp
│   ├── programacao_dinamica.cpp
│   ├── guloso.cpp               # heurística gulosa (densidade v/p)
│   └── aproximado.cpp           # 2-aproximação (guloso ou melhor item sozinho)
├── data/
│   ├── exemplo.txt              # instância de exemplo
│   ├── guloso_ruim.txt          # pior caso do guloso (obtém 2, ótimo é 100)
│   └── aproximado_justo.txt     # garantia de 1/2 é justa (obtém 52, ótimo é 100)
├── scripts/
│   └── graficos.py              # CSV -> gráficos
├── results/
│   ├── resultados.csv           # saída do benchmark (tempo e operações)
│   ├── resultados_heuristicas.csv  # tempos do guloso/aproximado (+ PD vetor) usados na seção 6.5
│   └── qualidade.csv            # saída do experimento de qualidade das heurísticas
├── docs/
│   ├── Documentação-Trabalho_parcial.md  # documentação (vira o PDF de entrega)
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
| `Benchmark + gráficos` | gera `results/resultados.csv`, `results/qualidade.csv` e os PNGs em `docs/img/` (~20 s) |

### Windows: PowerShell

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/main.cpp src/mochila.cpp src/divisao_conquista.cpp src/programacao_dinamica.cpp src/guloso.cpp src/aproximado.cpp -o build/mochila.exe
.\build\mochila.exe data\exemplo.txt

.\build\mochila.exe --benchmark results\resultados.csv
.\build\mochila.exe --qualidade results\qualidade.csv
python scripts\graficos.py results\resultados.csv docs\img results\qualidade.csv
```

### Linux / macOS / Git Bash

```bash
make run         # compila e resolve data/exemplo.txt
make benchmark   # CSVs + gráficos
```

Sem MinGW no Windows, dá para compilar pelo WSL: `wsl make run` (requer `g++` e `make` no Ubuntu).

## Formato da instância

```
n C V
p_1 v_1
...
p_n v_n
```

Exemplo (`data/exemplo.txt`): 4 itens, capacidade 7, alvo 9 → resposta SIM com os itens {2, 3}.

As heurísticas (guloso e aproximado) respondem `NAO SEI` quando não atingem o alvo: não achar uma solução com valor ≥ V não prova que ela não existe.

## Adicionando uma estratégia (para o grupo)

1. Declare a função em `include/mochila.hpp`, com a assinatura `Resultado resolverXxx(const Instancia&)`.
2. Implemente-a em `src/xxx.cpp`, incrementando `resultado.operacoes` na operação barômetro.
3. Registre-a em `estrategias()` no `src/main.cpp`, dizendo se é exata. As exatas precisam bater com o ótimo da PD. As heurísticas são conferidas quanto à viabilidade e à garantia de aproximação declarada (campo `garantia`) e entram também no experimento `--qualidade`.
4. Acrescente o arquivo às listas `NUCLEO` (Makefile) e `args` (`.vscode/tasks.json`).
As estratégias exponenciais (força bruta e backtracking) não vão aguentar o benchmark atual (n até 1024). Para elas, convém um experimento separado, com n pequeno.

## Gerar o PDF

Requer [pandoc](https://pandoc.org) e uma distribuição LaTeX (MiKTeX no Windows):

```bash
cd docs
pandoc Documentação-Trabalho_parcial.md -o trabalho.pdf --pdf-engine=xelatex -V geometry:margin=2.5cm -V lang=pt-BR
```
