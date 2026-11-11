/**
 * @file backtracking.cpp
 * @brief Mochila 0-1 por Backtracking com poda por limite superior.
 *
 * É a mesma árvore de decisão da força bruta — no nível k decide-se levar ou não o k-ésimo
 * item —, mas percorrida em profundidade e cortada sempre que o ramo não pode levar a nada
 * melhor do que a melhor solução já encontrada:
 *
 *   1. Poda por inviabilidade: o ramo "leva" só é gerado se o item couber na capacidade
 *      restante. Sem ela, o algoritmo gastaria tempo em subconjuntos pesados demais.
 *   2. Poda por limite superior: os itens são pré-ordenados por densidade v/p decrescente
 *      e, em cada nó, calcula-se o ótimo da mochila *fracionária* dos itens restantes, que
 *      é um limite superior do que ainda se pode ganhar (o fracionário relaxa a restrição
 *      0-1, logo seu ótimo é ≥ o ótimo inteiro). Se valorAtual + limite ≤ melhorValor, o
 *      ramo inteiro é abandonado.
 *
 * O limite é calculado em inteiros, arredondando a fração para cima: um limite
 * subestimado poderia podar o ramo que contém o ótimo. O produto cruzado da ordenação e o
 * numerador da fração usam 128 bits, como em guloso.cpp, para não estourar nem recorrer a
 * ponto flutuante.
 *
 * A ordenação por densidade também serve à poda: ela faz as soluções boas aparecerem no
 * começo da busca, o que eleva melhorValor rápido e torna a poda 2 agressiva desde os
 * primeiros níveis. O ramo "leva" é explorado antes do ramo "não leva" pelo mesmo motivo.
 *
 * Operação barômetro: um nó visitado (uma chamada recursiva). O pior caso é a árvore
 * inteira, 2^(n+1) - 1 nós, e cada nó custa O(n) por causa do cálculo do limite, o que dá
 * tempo O(n·2^n) — mesma ordem da força bruta. A contagem efetiva depende dos dados e, nas
 * instâncias medidas, fica ordens de grandeza abaixo desse limite.
 */
#include <algorithm>
#include <cstddef>

#include "mochila.hpp"

namespace mochila {
namespace {

// Inteiro de 128 bits (extensão do GCC/Clang); __extension__ evita o aviso de -Wpedantic.
__extension__ typedef __int128 Produto;

/** @brief Estado da busca em profundidade com poda. */
class Busca {
public:
    Busca(const Instancia& instancia, Contador& operacoes)
        : itens_(instancia.itens), operacoes_(operacoes) {
        ordem_.reserve(itens_.size());
        for (std::size_t i = 0; i < itens_.size(); ++i) ordem_.push_back(static_cast<int>(i));
        // Densidade decrescente por produto cruzado (v_a·p_b > v_b·p_a), sem ponto
        // flutuante; empates pelo índice, para a ordem ser total e o resultado determinístico.
        const std::vector<Item>& itens = itens_;
        std::sort(ordem_.begin(), ordem_.end(), [&itens](int a, int b) {
            const Item& x = itens[paraIndice(a)];
            const Item& y = itens[paraIndice(b)];
            const Produto esquerda = static_cast<Produto>(x.valor) * y.peso;
            const Produto direita = static_cast<Produto>(y.valor) * x.peso;
            if (esquerda != direita) return esquerda > direita;
            return a < b;
        });
    }

    /** @brief Explora o nó do nível k com `restante` de capacidade e `valorAtual` acumulado. */
    void explorar(std::size_t k, long long restante, Valor valorAtual) {
        ++operacoes_;
        if (valorAtual > melhorValor_) {
            melhorValor_ = valorAtual;
            melhorEscolha_ = atual_;
        }
        if (k == ordem_.size()) return;
        if (valorAtual + limiteSuperior(k, restante) <= melhorValor_) return;  // poda 2

        const Item& item = itens_[paraIndice(ordem_[k])];
        if (item.peso <= restante) {                                           // poda 1
            atual_.push_back(ordem_[k]);
            explorar(k + 1, restante - item.peso, valorAtual + item.valor);
            atual_.pop_back();
        }
        explorar(k + 1, restante, valorAtual);
    }

    Valor melhorValor() const { return melhorValor_; }
    const std::vector<int>& melhorEscolha() const { return melhorEscolha_; }

private:
    /**
     * @brief Ótimo da mochila fracionária dos itens k..n-1 com capacidade `restante`.
     * Leva inteiros enquanto couberem e frações apenas do primeiro que não couber, com o
     * teto da fração. O item fracionado tem peso > restante ≥ 0, logo peso > 0: a divisão
     * nunca é por zero (itens de peso 0 sempre cabem e entram inteiros).
     */
    Valor limiteSuperior(std::size_t k, long long restante) const {
        Valor limite = 0;
        for (std::size_t j = k; j < ordem_.size(); ++j) {
            const Item& item = itens_[paraIndice(ordem_[j])];
            if (item.peso <= restante) {
                restante -= item.peso;
                limite += item.valor;
            } else {
                const Produto numerador = static_cast<Produto>(restante) * item.valor;
                const Produto divisor = static_cast<Produto>(item.peso);
                limite += static_cast<Valor>((numerador + divisor - 1) / divisor);  // teto
                break;
            }
        }
        return limite;
    }

    const std::vector<Item>& itens_;
    Contador& operacoes_;
    std::vector<int> ordem_;          ///< Índices originais em ordem de densidade decrescente.
    std::vector<int> atual_;          ///< Itens do caminho da raiz até o nó atual.
    std::vector<int> melhorEscolha_;  ///< Melhor caminho completo encontrado.
    Valor melhorValor_ = 0;
};

}  // namespace

Resultado resolverBacktracking(const Instancia& instancia) {
    Resultado resultado;
    Busca busca(instancia, resultado.operacoes);
    busca.explorar(0, instancia.capacidade, 0);

    resultado.melhorValor = busca.melhorValor();
    resultado.selecionados = busca.melhorEscolha();
    std::sort(resultado.selecionados.begin(), resultado.selecionados.end());
    resultado.atingeAlvo = resultado.melhorValor >= instancia.alvo;
    return resultado;
}

}  // namespace mochila
