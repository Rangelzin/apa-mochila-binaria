/**
 * @file programacao_dinamica.cpp
 * @brief Mochila 0-1 por Programação Dinâmica (bottom-up).
 *
 * Subestrutura ótima: seja M[i][c] o maior valor usando os i primeiros itens com
 * capacidade c. Então
 *   M[0][c] = 0
 *   M[i][c] = M[i-1][c]                                   se p_i > c
 *   M[i][c] = max(M[i-1][c], M[i-1][c - p_i] + v_i)       se p_i ≤ c
 *
 * A recursão ingênua dessa equação recalcula os mesmos M[i][c] várias vezes
 * (subproblemas sobrepostos, até 2^n chamadas); a tabela calcula cada um uma única vez.
 *
 * Operação barômetro: cálculo de uma célula M[i][c], i ≥ 1. Total exato: n·(C+1).
 */
#include <algorithm>
#include <cstddef>

#include "mochila.hpp"

namespace mochila {
namespace {

/** @brief Tabela (n+1)×(C+1) armazenada em vetor contíguo (melhor localidade de cache). */
class Tabela {
public:
    Tabela(std::size_t linhas, std::size_t colunas)
        : colunas_(colunas), dados_(linhas * colunas, 0) {}

    Valor& em(std::size_t i, std::size_t c) { return dados_[i * colunas_ + c]; }
    Valor em(std::size_t i, std::size_t c) const { return dados_[i * colunas_ + c]; }

private:
    std::size_t colunas_;
    std::vector<Valor> dados_;
};

/** @brief Percorre a tabela de trás para frente e recupera os itens escolhidos. O(n). */
std::vector<int> reconstruirSolucao(const Tabela& tabela, const std::vector<Item>& itens, std::size_t capacidade) {
    std::vector<int> selecionados;
    std::size_t c = capacidade;
    for (std::size_t i = itens.size(); i >= 1; --i) {
        if (tabela.em(i, c) != tabela.em(i - 1, c)) {  // valor mudou ⇒ item i foi usado
            selecionados.push_back(static_cast<int>(i - 1));
            c -= paraIndice(itens[i - 1].peso);        // seguro: o item só foi usado se coube em c
        }
    }
    std::reverse(selecionados.begin(), selecionados.end());
    return selecionados;
}

}  // namespace

Resultado resolverProgramacaoDinamica(const Instancia& instancia) {
    const std::vector<Item>& itens = instancia.itens;
    const std::size_t n = itens.size();
    const std::size_t capacidade = paraIndice(instancia.capacidade);
    Tabela tabela(n + 1, capacidade + 1);  // linha 0 já é zero

    Resultado resultado;
    for (std::size_t i = 1; i <= n; ++i) {                                    // linha 1
        const std::size_t peso = paraIndice(itens[i - 1].peso);
        const Valor valor = itens[i - 1].valor;
        for (std::size_t c = 0; c <= capacidade; ++c) {                       // linha 2
            ++resultado.operacoes;
            tabela.em(i, c) = tabela.em(i - 1, c);                            // linha 3: não leva
            if (peso <= c) {                                                  // linha 4
                tabela.em(i, c) = std::max(tabela.em(i, c),                   // linha 5: leva
                                           tabela.em(i - 1, c - peso) + valor);
            }
        }
    }

    resultado.melhorValor = tabela.em(n, capacidade);
    resultado.atingeAlvo = resultado.melhorValor >= instancia.alvo;
    resultado.selecionados = reconstruirSolucao(tabela, itens, capacidade);
    return resultado;
}

Resultado resolverProgramacaoDinamicaVetor(const Instancia& instancia) {
    const std::size_t capacidade = paraIndice(instancia.capacidade);
    std::vector<Valor> melhor(capacidade + 1, 0);

    Resultado resultado;
    for (const Item& item : instancia.itens) {
        const std::size_t peso = paraIndice(item.peso);
        // Ordem decrescente de c: melhor[c - peso] ainda é da "linha anterior",
        // garantindo que cada item seja usado no máximo uma vez.
        // "c-- > peso" percorre c = C, C-1, ..., peso sem underflow do size_t (inclusive com peso 0).
        for (std::size_t c = capacidade + 1; c-- > peso;) {
            ++resultado.operacoes;
            melhor[c] = std::max(melhor[c], melhor[c - peso] + item.valor);
        }
    }

    resultado.melhorValor = melhor[capacidade];
    resultado.atingeAlvo = resultado.melhorValor >= instancia.alvo;
    return resultado;
}

}  // namespace mochila
