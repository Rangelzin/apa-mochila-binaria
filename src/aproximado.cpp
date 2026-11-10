/**
 * @file aproximado.cpp
 * @brief Mochila 0-1 por algoritmo 2-aproximado (guloso modificado).
 *
 * Devolve a melhor entre duas soluções viáveis:
 *   (a) a solução do guloso por densidade (guloso.cpp);
 *   (b) o item de maior valor que cabe sozinho (p_i ≤ C).
 *
 * Garantia: melhorValor ≥ OPT / 2.
 *   Itens com p_i > C não entram em solução nenhuma; descarte-os (o guloso também os
 *   pula). Ordene os restantes por densidade e seja k o primeiro que o guloso não
 *   consegue levar. Os itens 1..k-1 mais o item k inteiro valem pelo menos o ótimo
 *   fracionário, que é ≥ OPT. Logo valor(1..k-1) + v_k ≥ OPT, e o maior dos dois termos
 *   é ≥ OPT/2. O guloso vale pelo menos valor(1..k-1), e o item mais valioso que cabe
 *   sozinho vale pelo menos v_k, pois p_k ≤ C. Se o guloso leva todos, ele já é ótimo.
 *
 * O caso (b) corrige exatamente o pior caso do guloso: um item pequeno e muito denso
 * que "rouba" a capacidade de um item grande e valioso.
 *
 * Operação barômetro: a do guloso mais uma inspeção por item na busca de (b).
 */
#include <cstddef>

#include "mochila.hpp"

namespace mochila {

Resultado resolverAproximado(const Instancia& instancia) {
    Resultado resultado = resolverGuloso(instancia);

    // (b) item mais valioso que cabe sozinho.
    int melhorItem = -1;
    for (std::size_t i = 0; i < instancia.itens.size(); ++i) {
        ++resultado.operacoes;
        const Item& item = instancia.itens[i];
        if (item.peso <= instancia.capacidade &&
            (melhorItem < 0 || item.valor > instancia.itens[paraIndice(melhorItem)].valor)) {
            melhorItem = static_cast<int>(i);
        }
    }

    if (melhorItem >= 0 && instancia.itens[paraIndice(melhorItem)].valor > resultado.melhorValor) {
        resultado.melhorValor = instancia.itens[paraIndice(melhorItem)].valor;
        resultado.selecionados = {melhorItem};
    }

    resultado.atingeAlvo = resultado.melhorValor >= instancia.alvo;
    return resultado;
}

}  // namespace mochila
