/**
 * @file mochila.cpp
 * @brief Leitura, geração e verificação de instâncias (código comum às estratégias).
 */
#include "mochila.hpp"

#include <random>
#include <stdexcept>
#include <string>

namespace mochila {

Instancia lerInstancia(std::istream& entrada) {
    int n = 0;
    Instancia instancia{};
    if (!(entrada >> n >> instancia.capacidade >> instancia.alvo)) {
        throw std::runtime_error("cabeçalho inválido: esperado 'n C V'");
    }
    if (n < 0 || instancia.capacidade < 0) {
        throw std::runtime_error("n e C devem ser não negativos");
    }
    instancia.itens.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        Item item{};
        if (!(entrada >> item.peso >> item.valor)) {
            throw std::runtime_error("item " + std::to_string(i + 1) + " ausente ou malformado");
        }
        if (item.peso < 0 || item.valor < 0) {
            throw std::runtime_error("item " + std::to_string(i + 1) + " com peso ou valor negativo");
        }
        instancia.itens.push_back(item);
    }
    return instancia;
}

Instancia gerarInstancia(int n, int capacidade, unsigned semente) {
    std::mt19937 gerador(semente);
    std::uniform_int_distribution<int> distPeso(1, capacidade > 0 ? capacidade : 1);
    std::uniform_int_distribution<int> distValor(1, 1000);

    Instancia instancia{{}, capacidade, 0};
    instancia.itens.reserve(static_cast<std::size_t>(n));
    Valor somaValores = 0;
    for (int i = 0; i < n; ++i) {
        Item item{distPeso(gerador), distValor(gerador)};
        somaValores += item.valor;
        instancia.itens.push_back(item);
    }
    instancia.alvo = somaValores / 2;
    return instancia;
}

bool verificarCertificado(const Instancia& instancia, const std::vector<int>& selecionados) {
    const std::size_t n = instancia.itens.size();
    std::vector<bool> usado(n, false);
    long long pesoTotal = 0;
    Valor valorTotal = 0;
    for (int indice : selecionados) {
        if (indice < 0 || paraIndice(indice) >= n || usado[paraIndice(indice)]) {
            return false;  // índice inválido ou item repetido (viola o "0-1")
        }
        const std::size_t i = paraIndice(indice);
        usado[i] = true;
        pesoTotal += instancia.itens[i].peso;
        valorTotal += instancia.itens[i].valor;
    }
    return pesoTotal <= instancia.capacidade && valorTotal >= instancia.alvo;
}

}  // namespace mochila
