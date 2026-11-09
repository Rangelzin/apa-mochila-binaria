/**
 * @file mochila.hpp
 * @brief Tipos e contratos compartilhados do Problema da Mochila Binária (0-1 Knapsack).
 *
 * Versão de decisão: dados n itens (peso p_i, valor v_i), capacidade C e valor-alvo V,
 * existe S ⊆ {1..n} com Σ p_i ≤ C e Σ v_i ≥ V?
 *
 * Toda estratégia resolve a versão de otimização (maior valor que cabe em C) e
 * responde à decisão comparando esse valor com V.
 */
#ifndef MOCHILA_HPP
#define MOCHILA_HPP

#include <cstddef>
#include <cstdint>
#include <istream>
#include <vector>

namespace mochila {

using Valor = std::int64_t;
using Contador = std::uint64_t;

/**
 * @brief Converte um inteiro não negativo (peso, capacidade, índice) para índice de vetor.
 * Centraliza o cast e mantém o código limpo sob -Wsign-conversion.
 * @pre x ≥ 0 (garantido por lerInstancia/gerarInstancia).
 */
inline std::size_t paraIndice(int x) { return static_cast<std::size_t>(x); }

/** @brief Item da mochila: peso e valor, ambos não negativos. */
struct Item {
    int peso;
    Valor valor;
};

/** @brief Instância do problema de decisão. */
struct Instancia {
    std::vector<Item> itens;
    int capacidade;
    Valor alvo;
};

/** @brief Saída padrão de qualquer estratégia. */
struct Resultado {
    Valor melhorValor = 0;          ///< Maior valor total com peso ≤ C.
    bool atingeAlvo = false;        ///< Resposta da versão de decisão (melhorValor ≥ V).
    std::vector<int> selecionados;  ///< Certificado (índices 0-based); vazio se a estratégia não reconstrói.
    Contador operacoes = 0;         ///< Execuções da operação barômetro da estratégia.
};

/**
 * @brief Lê uma instância no formato:
 *   n C V
 *   p_1 v_1
 *   ...
 *   p_n v_n
 * @throws std::runtime_error se o formato ou os valores forem inválidos.
 */
Instancia lerInstancia(std::istream& entrada);

/**
 * @brief Gera instância aleatória reprodutível.
 * Pesos uniformes em [1, C], valores uniformes em [1, 1000], alvo = metade da soma dos valores.
 */
Instancia gerarInstancia(int n, int capacidade, unsigned semente);

/**
 * @brief Verificador polinomial O(n) do certificado (base da prova de pertinência a NP).
 * @return true se os índices são distintos, válidos, cabem em C e somam valor ≥ V.
 */
bool verificarCertificado(const Instancia& instancia, const std::vector<int>& selecionados);

/** @brief Divisão e conquista sobre o conjunto de itens. Tempo Θ(n·C²), espaço Θ(C·log n). */
Resultado resolverDivisaoConquista(const Instancia& instancia);

/** @brief Programação dinâmica com tabela (n+1)×(C+1). Tempo Θ(n·C), espaço Θ(n·C). */
Resultado resolverProgramacaoDinamica(const Instancia& instancia);

/** @brief Programação dinâmica com vetor único. Tempo O(n·C), espaço Θ(C); não reconstrói solução. */
Resultado resolverProgramacaoDinamicaVetor(const Instancia& instancia);

}  // namespace mochila

#endif  // MOCHILA_HPP
