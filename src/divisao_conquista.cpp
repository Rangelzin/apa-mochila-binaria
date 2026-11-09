/**
 * @file divisao_conquista.cpp
 * @brief Mochila 0-1 por Divisão e Conquista sobre o conjunto de itens.
 *
 * Ideia:
 *   - Dividir: separa os itens [ini, fim) em duas metades.
 *   - Conquistar: resolve cada metade recursivamente, obtendo para ela o vetor
 *     melhor[c] = maior valor usando só itens da metade com peso total ≤ c, para c = 0..C.
 *   - Combinar: a capacidade c é repartida entre as metades (a para a esquerda, c - a
 *     para a direita), então resultado[c] = max_{0≤a≤c} (esq[a] + dir[c - a]).
 *
 * As metades são disjuntas, logo os subproblemas são independentes (sem sobreposição),
 * o que caracteriza D&C e o diferencia da Programação Dinâmica.
 *
 * Operação barômetro: uma avaliação de candidato (atribuição no caso base ou
 * soma+comparação na combinação). Total exato para n ≥ 1:
 *   n·(C+1) + (n-1)·(C+1)(C+2)/2
 */
#include <algorithm>
#include <cstddef>

#include "mochila.hpp"

namespace mochila {
namespace {

using Perfil = std::vector<Valor>;  // perfil[c] = melhor valor com capacidade ≤ c

/** @brief Caso base: um único item cabe (valor) ou não cabe (0) em cada capacidade. */
Perfil resolverItemUnico(const Item& item, std::size_t capacidade, Contador& operacoes) {
    const std::size_t peso = paraIndice(item.peso);
    Perfil perfil(capacidade + 1, 0);
    for (std::size_t c = 0; c <= capacidade; ++c) {
        ++operacoes;
        perfil[c] = (peso <= c) ? item.valor : 0;
    }
    return perfil;
}

/** @brief Combina dois perfis disjuntos por convolução (max, +). Custo (C+1)(C+2)/2. */
Perfil combinar(const Perfil& esquerda, const Perfil& direita, std::size_t capacidade, Contador& operacoes) {
    Perfil combinado(capacidade + 1, 0);
    for (std::size_t c = 0; c <= capacidade; ++c) {
        Valor melhor = 0;
        for (std::size_t a = 0; a <= c; ++a) {
            ++operacoes;
            melhor = std::max(melhor, esquerda[a] + direita[c - a]);
        }
        combinado[c] = melhor;
    }
    return combinado;
}

/** @brief Resolve o subconjunto de itens [inicio, fim), com fim - inicio ≥ 1. */
Perfil resolverIntervalo(const std::vector<Item>& itens, std::size_t inicio, std::size_t fim,
                         std::size_t capacidade, Contador& operacoes) {
    if (fim - inicio == 1) {
        return resolverItemUnico(itens[inicio], capacidade, operacoes);
    }
    const std::size_t meio = inicio + (fim - inicio) / 2;
    const Perfil esquerda = resolverIntervalo(itens, inicio, meio, capacidade, operacoes);
    const Perfil direita = resolverIntervalo(itens, meio, fim, capacidade, operacoes);
    return combinar(esquerda, direita, capacidade, operacoes);
}

}  // namespace

Resultado resolverDivisaoConquista(const Instancia& instancia) {
    Resultado resultado;
    if (!instancia.itens.empty()) {
        const std::size_t capacidade = paraIndice(instancia.capacidade);
        const Perfil perfil = resolverIntervalo(instancia.itens, 0, instancia.itens.size(),
                                                capacidade, resultado.operacoes);
        resultado.melhorValor = perfil[capacidade];
    }
    resultado.atingeAlvo = resultado.melhorValor >= instancia.alvo;
    return resultado;
}

}  // namespace mochila
