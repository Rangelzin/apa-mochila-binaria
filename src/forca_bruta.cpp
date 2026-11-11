/**
 * @file forca_bruta.cpp
 * @brief Mochila 0-1 por Força Bruta (enumeração exaustiva de subconjuntos).
 *
 * Ideia: cada subconjunto S ⊆ {1..n} corresponde a uma máscara de n bits, em que o bit i
 * ligado significa "levo o item i". O algoritmo percorre as 2^n máscaras, soma o peso e o
 * valor dos itens marcados e guarda a melhor máscara viável (peso total ≤ C).
 *
 * Não há poda alguma, de propósito: a estratégia serve de baseline exato e de medida do
 * tamanho do espaço de busca. Podar essa mesma árvore de decisão é exatamente o que o
 * backtracking faz (backtracking.cpp), e a comparação entre as duas contagens de operações
 * mostra quanto a poda economiza.
 *
 * Como o espaço de busca é percorrido inteiro, o custo não depende dos valores da entrada:
 * duas instâncias com o mesmo n custam o mesmo.
 *
 * Operação barômetro: inspeção de um item dentro de um candidato (o teste de um bit da
 * máscara). Total exato: n·2^n.
 */
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "mochila.hpp"

namespace mochila {
namespace {

/** @brief Máscara de bits é um uint64: no máximo 63 itens (2^63 cabe sem estouro). */
constexpr std::size_t kMaxItens = 63;

/** @brief Bit i da máscara. */
inline std::uint64_t bit(std::size_t i) { return std::uint64_t{1} << i; }

}  // namespace

Resultado resolverForcaBruta(const Instancia& instancia) {
    const std::vector<Item>& itens = instancia.itens;
    const std::size_t n = itens.size();
    if (n > kMaxItens) {
        throw std::domain_error("forca bruta: n = " + std::to_string(n) + " excede o limite de " +
                                std::to_string(kMaxItens) + " itens (2^n)");
    }

    Resultado resultado;
    const std::uint64_t candidatos = bit(n);  // 2^n máscaras, de 0 a 2^n - 1
    std::uint64_t melhorMascara = 0;

    for (std::uint64_t mascara = 0; mascara < candidatos; ++mascara) {
        long long peso = 0;
        Valor valor = 0;
        for (std::size_t i = 0; i < n; ++i) {
            ++resultado.operacoes;
            if (mascara & bit(i)) {
                peso += itens[i].peso;
                valor += itens[i].valor;
            }
        }
        if (peso <= instancia.capacidade && valor > resultado.melhorValor) {
            resultado.melhorValor = valor;
            melhorMascara = mascara;
        }
    }

    for (std::size_t i = 0; i < n; ++i) {
        if (melhorMascara & bit(i)) resultado.selecionados.push_back(static_cast<int>(i));
    }
    resultado.atingeAlvo = resultado.melhorValor >= instancia.alvo;
    return resultado;
}

}  // namespace mochila
