/**
 * @file testes.cpp
 * @brief Testes: casos de borda + comparação aleatória contra um oráculo de força bruta (2^n).
 * O oráculo existe só para validação; a força bruta "oficial" é de outro integrante.
 */
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

#include "mochila.hpp"

using namespace mochila;

namespace {

Valor oraculoForcaBruta(const Instancia& instancia) {
    const std::size_t n = instancia.itens.size();
    Valor melhor = 0;
    for (unsigned long mascara = 0; mascara < (1UL << n); ++mascara) {
        long long peso = 0;
        Valor valor = 0;
        for (std::size_t i = 0; i < n; ++i) {
            if (mascara & (1UL << i)) {
                peso += instancia.itens[i].peso;
                valor += instancia.itens[i].valor;
            }
        }
        if (peso <= instancia.capacidade) melhor = std::max(melhor, valor);
    }
    return melhor;
}

void verificarTodas(const Instancia& instancia, Valor esperado) {
    const Resultado dc = resolverDivisaoConquista(instancia);
    const Resultado pd = resolverProgramacaoDinamica(instancia);
    const Resultado pv = resolverProgramacaoDinamicaVetor(instancia);
    assert(dc.melhorValor == esperado);
    assert(pd.melhorValor == esperado);
    assert(pv.melhorValor == esperado);
    assert(dc.atingeAlvo == (esperado >= instancia.alvo));
    assert(pd.atingeAlvo == dc.atingeAlvo && pv.atingeAlvo == dc.atingeAlvo);
    // O certificado da PD prova "SIM" sempre que a resposta é SIM.
    if (pd.atingeAlvo) assert(verificarCertificado(instancia, pd.selecionados));
}

void testarBordas() {
    verificarTodas({{}, 10, 0}, 0);                           // sem itens
    verificarTodas({{{5, 10}}, 0, 1}, 0);                     // capacidade zero
    verificarTodas({{{11, 99}}, 10, 1}, 0);                   // item não cabe
    verificarTodas({{{10, 7}}, 10, 7}, 7);                    // cabe exatamente
    verificarTodas({{{0, 5}, {3, 4}}, 2, 5}, 5);              // item de peso zero
    verificarTodas({{{1, 1}, {3, 4}, {4, 5}, {5, 7}}, 7, 9}, 9);  // exemplo clássico
}

void testarLeitura() {
    std::istringstream entrada("2 5 3\n2 3\n3 4\n");
    const Instancia instancia = lerInstancia(entrada);
    assert(instancia.itens.size() == 2 && instancia.capacidade == 5 && instancia.alvo == 3);
    std::istringstream invalida("2 5 3\n2 3\n");
    bool lancou = false;
    try { lerInstancia(invalida); } catch (const std::runtime_error&) { lancou = true; }
    assert(lancou);
}

void testarAleatorio() {
    for (unsigned semente = 1; semente <= 300; ++semente) {
        const int n = static_cast<int>(semente % 13);  // 0..12
        const int capacidade = static_cast<int>(semente % 40);
        const Instancia instancia = gerarInstancia(n, capacidade, semente);
        verificarTodas(instancia, oraculoForcaBruta(instancia));
    }
}

void testarContagemOperacoes() {
    const Instancia instancia = gerarInstancia(37, 50, 7);
    const Contador n = 37, c = 50;
    assert(resolverDivisaoConquista(instancia).operacoes == n * (c + 1) + (n - 1) * (c + 1) * (c + 2) / 2);
    assert(resolverProgramacaoDinamica(instancia).operacoes == n * (c + 1));
}

}  // namespace

int main() {
    testarBordas();
    testarLeitura();
    testarAleatorio();
    testarContagemOperacoes();
    std::cout << "todos os testes passaram\n";
    return 0;
}
