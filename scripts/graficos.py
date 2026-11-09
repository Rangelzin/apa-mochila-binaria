"""Gera os gráficos de tempo x n e tempo x C a partir do CSV do benchmark.

Uso: python scripts/graficos.py [results/resultados.csv] [docs/img]
"""
import csv
import sys
from pathlib import Path
from collections import defaultdict

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROTULOS = {"divisao_conquista": "Divisão e Conquista", "pd_tabela": "PD (tabela)", "pd_vetor": "PD (vetor)"}


def carregar(caminho):
    series = defaultdict(lambda: defaultdict(list))
    with open(caminho, newline="") as arquivo:
        for linha in csv.DictReader(arquivo):
            eixo = int(linha["n"]) if linha["experimento"] == "varia_n" else int(linha["C"])
            series[linha["experimento"]][linha["estrategia"]].append((eixo, float(linha["tempo_ms"])))
    return series


def plotar(series, experimento, rotulo_x, saida):
    fig, ax = plt.subplots(figsize=(6, 4))
    for estrategia, pontos in series[experimento].items():
        xs, ys = zip(*sorted(pontos))
        ax.plot(xs, ys, marker="o", label=ROTULOS.get(estrategia, estrategia))
    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xlabel(rotulo_x)
    ax.set_ylabel("tempo (ms, mediana de 5)")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend()
    fig.tight_layout()
    fig.savefig(saida, dpi=150)


if __name__ == "__main__":
    entrada = sys.argv[1] if len(sys.argv) > 1 else "results/resultados.csv"
    saida = Path(sys.argv[2] if len(sys.argv) > 2 else "docs/img")
    saida.mkdir(parents=True, exist_ok=True)
    dados = carregar(entrada)
    plotar(dados, "varia_n", "n (C = 1000)", saida / "grafico_varia_n.png")
    plotar(dados, "varia_C", "C (n = 100)", saida / "grafico_varia_C.png")
