"""Gera os gráficos a partir dos CSVs do benchmark e do experimento de qualidade.

Uso: python scripts/graficos.py [results/resultados.csv] [docs/img] [results/qualidade.csv]

- resultados.csv -> grafico_varia_n.png, grafico_varia_C.png (tempo)
- qualidade.csv  -> grafico_qualidade.png (valor heurístico / ótimo); pulado se o arquivo não existir
"""
import csv
import sys
from pathlib import Path
from collections import defaultdict

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROTULOS = {
    "divisao_conquista": "Divisão e Conquista",
    "pd_tabela": "PD (tabela)",
    "pd_vetor": "PD (vetor)",
    "guloso": "Guloso",
    "aproximado": "2-aproximado",
}
TITULOS_FAMILIA = {"aleatoria": "Instâncias aleatórias", "correlacionada": "Instâncias correlacionadas"}


def carregar(caminho):
    series = defaultdict(lambda: defaultdict(list))
    with open(caminho, newline="") as arquivo:
        for linha in csv.DictReader(arquivo):
            eixo = int(linha["n"]) if linha["experimento"] == "varia_n" else int(linha["C"])
            series[linha["experimento"]][linha["estrategia"]].append((eixo, float(linha["tempo_ms"])))
    return series


def plotar(series, experimento, rotulo_x, saida):
    fig, ax = plt.subplots(figsize=(7.5, 4))
    for estrategia, pontos in series[experimento].items():
        xs, ys = zip(*sorted(pontos))
        ax.plot(xs, ys, marker="o", label=ROTULOS.get(estrategia, estrategia))
    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xlabel(rotulo_x)
    ax.set_ylabel("tempo (ms, mediana de 5)")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(loc="upper left", bbox_to_anchor=(1.02, 1), fontsize=9)
    fig.tight_layout()
    fig.savefig(saida, dpi=150)
    plt.close(fig)


def plotar_qualidade(caminho, saida):
    """Um painel por família: razão média (linha cheia) e mínima (tracejada) contra n."""
    dados = defaultdict(lambda: defaultdict(list))
    with open(caminho, newline="") as arquivo:
        for linha in csv.DictReader(arquivo):
            dados[linha["familia"]][linha["estrategia"]].append(
                (int(linha["n"]), float(linha["razao_media"]), float(linha["razao_min"]))
            )

    familias = list(dados)
    fig, eixos = plt.subplots(1, len(familias), figsize=(6 * len(familias), 4), sharey=True, squeeze=False)
    for ax, familia in zip(eixos[0], familias):
        for estrategia, pontos in dados[familia].items():
            ns, medias, minimos = zip(*sorted(pontos))
            rotulo = ROTULOS.get(estrategia, estrategia)
            (linha,) = ax.plot(ns, medias, marker="o", label=f"{rotulo} (média)")
            ax.plot(ns, minimos, marker="x", linestyle="--", color=linha.get_color(), label=f"{rotulo} (pior caso)")
        ax.axhline(0.5, color="gray", linewidth=1, linestyle=":", label="garantia do 2-aproximado")
        ax.set_xscale("log")
        ax.set_ylim(0, 1.05)
        ax.set_xlabel("n (C = 1000, 200 instâncias por ponto)")
        ax.set_title(TITULOS_FAMILIA.get(familia, familia))
        ax.grid(True, which="both", alpha=0.3)
    eixos[0][0].set_ylabel("valor obtido / ótimo")
    eixos[0][-1].legend(loc="lower right", fontsize=8)
    fig.tight_layout()
    fig.savefig(saida, dpi=150)
    plt.close(fig)


if __name__ == "__main__":
    entrada = sys.argv[1] if len(sys.argv) > 1 else "results/resultados.csv"
    saida = Path(sys.argv[2] if len(sys.argv) > 2 else "docs/img")
    qualidade = Path(sys.argv[3] if len(sys.argv) > 3 else "results/qualidade.csv")
    saida.mkdir(parents=True, exist_ok=True)
    dados = carregar(entrada)
    plotar(dados, "varia_n", "n (C = 1000)", saida / "grafico_varia_n.png")
    plotar(dados, "varia_C", "C (n = 100)", saida / "grafico_varia_C.png")
    if qualidade.exists():
        plotar_qualidade(qualidade, saida / "grafico_qualidade.png")
