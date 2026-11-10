/**
 * @file guloso.cpp
 * @brief Mochila 0-1 por estratégia gulosa (heurística, sem garantia de otimalidade).
 *
 * Critério guloso: densidade v_i / p_i, a maior primeiro. Os itens são percorridos nessa
 * ordem e cada um entra se ainda couber na capacidade restante; os que não cabem são
 * pulados (a varredura continua, pois um item menor adiante ainda pode caber).
 *
 * Pré-processamento:
 *   - itens com valor 0 são descartados (nunca melhoram a solução);
 *   - itens com peso 0 e valor > 0 entram sempre (densidade infinita);
 *   - os demais têm p_i > 0 e v_i > 0 e são comparados por produto cruzado
 *     (v_i·p_j > v_j·p_i), o que evita ponto flutuante. Empates são desfeitos pelo
 *     índice, então a ordem é total e o resultado é determinístico.
 *
 * O guloso é ótimo para a mochila *fracionária*, mas não para a 0-1: na instância
 * data/guloso_ruim.txt ele obtém 2 quando o ótimo é C.
 *
 * Operação barômetro: uma comparação entre itens na ordenação ou a inspeção de um item
 * (no filtro ou na varredura). Total O(n log n), dependente dos dados.
 */
#include <algorithm>
#include <cstddef>

#include "mochila.hpp"

namespace mochila {
namespace {

// Inteiro de 128 bits (extensão do GCC/Clang); __extension__ evita o aviso de -Wpedantic.
__extension__ typedef __int128 Produto;

}  // namespace

Resultado resolverGuloso(const Instancia& instancia) {
    const std::vector<Item>& itens = instancia.itens;
    Resultado resultado;

    // Filtro: separa os itens de peso 0 (sempre entram) dos candidatos à ordenação.
    std::vector<int> candidatos;
    candidatos.reserve(itens.size());
    std::vector<int> pesoZero;
    for (std::size_t i = 0; i < itens.size(); ++i) {
        ++resultado.operacoes;
        if (itens[i].valor == 0) continue;
        if (itens[i].peso == 0) {
            pesoZero.push_back(static_cast<int>(i));
        } else {
            candidatos.push_back(static_cast<int>(i));
        }
    }

    // Ordenação por densidade decrescente. O produto cruzado é feito em 128 bits:
    // valor (int64) × peso (int) cabe sem estouro para qualquer entrada aceita.
    Contador& operacoes = resultado.operacoes;
    std::sort(candidatos.begin(), candidatos.end(), [&itens, &operacoes](int a, int b) {
        ++operacoes;
        const Item& x = itens[paraIndice(a)];
        const Item& y = itens[paraIndice(b)];
        const Produto esquerda = static_cast<Produto>(x.valor) * y.peso;
        const Produto direita = static_cast<Produto>(y.valor) * x.peso;
        if (esquerda != direita) return esquerda > direita;
        return a < b;
    });

    long long restante = instancia.capacidade;
    for (int i : pesoZero) {
        resultado.selecionados.push_back(i);
        resultado.melhorValor += itens[paraIndice(i)].valor;
    }
    for (int i : candidatos) {
        ++resultado.operacoes;
        const Item& item = itens[paraIndice(i)];
        if (item.peso <= restante) {
            restante -= item.peso;
            resultado.melhorValor += item.valor;
            resultado.selecionados.push_back(i);
        }
    }

    std::sort(resultado.selecionados.begin(), resultado.selecionados.end());
    resultado.atingeAlvo = resultado.melhorValor >= instancia.alvo;
    return resultado;
}

}  // namespace mochila
