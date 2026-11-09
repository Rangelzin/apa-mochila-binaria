/**
 * @file main.cpp
 * @brief Driver: resolve uma instância de arquivo ou roda o benchmark (tempo + operações).
 *
 * Uso:
 *   ./build/mochila data/exemplo.txt                      resolve e imprime resultado de cada estratégia
 *   ./build/mochila --benchmark results/resultados.csv    executa os experimentos e grava CSV
 */
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "mochila.hpp"

namespace {

using mochila::Contador;
using mochila::Instancia;
using mochila::Resultado;

struct Estrategia {
    std::string nome;
    std::function<Resultado(const Instancia&)> resolver;
    std::function<Contador(int n, int capacidade)> operacoesTeoricas;  // 0 = depende dos dados
};

std::vector<Estrategia> estrategias() {
    return {
        {"divisao_conquista", mochila::resolverDivisaoConquista,
         [](int n, int c) -> Contador {
             if (n == 0) return 0;
             const Contador cc = static_cast<Contador>(c);
             return static_cast<Contador>(n) * (cc + 1) +
                    static_cast<Contador>(n - 1) * (cc + 1) * (cc + 2) / 2;
         }},
        {"pd_tabela", mochila::resolverProgramacaoDinamica,
         [](int n, int c) -> Contador { return static_cast<Contador>(n) * (static_cast<Contador>(c) + 1); }},
        {"pd_vetor", mochila::resolverProgramacaoDinamicaVetor,
         [](int, int) -> Contador { return 0; }},
    };
}

/** @brief Mede a mediana de `repeticoes` execuções, em milissegundos. */
double medirMedianaMs(const Estrategia& estrategia, const Instancia& instancia, int repeticoes,
                      Resultado& ultimo) {
    std::vector<double> tempos;
    tempos.reserve(static_cast<std::size_t>(repeticoes));
    for (int r = 0; r < repeticoes; ++r) {
        const auto inicio = std::chrono::steady_clock::now();
        ultimo = estrategia.resolver(instancia);
        const auto fim = std::chrono::steady_clock::now();
        tempos.push_back(std::chrono::duration<double, std::milli>(fim - inicio).count());
    }
    std::nth_element(tempos.begin(), tempos.begin() + repeticoes / 2, tempos.end());
    return tempos[static_cast<std::size_t>(repeticoes / 2)];
}

void imprimirResultado(const std::string& nome, const Resultado& r, const Instancia& instancia) {
    std::cout << nome << ": melhor valor = " << r.melhorValor
              << " | resposta (>= " << instancia.alvo << ") = " << (r.atingeAlvo ? "SIM" : "NAO")
              << " | operacoes = " << r.operacoes;
    if (!r.selecionados.empty()) {
        std::cout << " | itens = {";
        for (std::size_t k = 0; k < r.selecionados.size(); ++k) {
            std::cout << (k ? ", " : "") << r.selecionados[k] + 1;  // 1-based na saída
        }
        std::cout << "} | certificado valido = "
                  << (mochila::verificarCertificado(instancia, r.selecionados) ? "sim" : "nao");
    }
    std::cout << '\n';
}

int resolverArquivo(const std::string& caminho) {
    std::ifstream arquivo(caminho);
    if (!arquivo) throw std::runtime_error("não foi possível abrir " + caminho);
    const Instancia instancia = mochila::lerInstancia(arquivo);
    std::cout << "n = " << instancia.itens.size() << ", C = " << instancia.capacidade
              << ", V = " << instancia.alvo << "\n";
    for (const Estrategia& e : estrategias()) {
        imprimirResultado(e.nome, e.resolver(instancia), instancia);
    }
    return 0;
}

struct Ponto {
    std::string experimento;
    int n;
    int capacidade;
};

std::vector<Ponto> pontosDoExperimento() {
    std::vector<Ponto> pontos;
    for (int n : {16, 32, 64, 128, 256, 512, 1024}) pontos.push_back({"varia_n", n, 1000});
    for (int c : {125, 250, 500, 1000, 2000, 4000, 8000}) pontos.push_back({"varia_C", 100, c});
    return pontos;
}

int rodarBenchmark(const std::string& caminhoCsv) {
    constexpr int kRepeticoes = 5;
    constexpr unsigned kSemente = 42;
    std::ofstream csv(caminhoCsv);
    if (!csv) throw std::runtime_error("não foi possível criar " + caminhoCsv);
    csv << "experimento,estrategia,n,C,tempo_ms,operacoes,operacoes_teoricas,melhor_valor\n";

    for (const Ponto& p : pontosDoExperimento()) {
        const Instancia instancia = mochila::gerarInstancia(p.n, p.capacidade, kSemente);
        mochila::Valor referencia = -1;
        for (const Estrategia& e : estrategias()) {
            Resultado r;
            const double ms = medirMedianaMs(e, instancia, kRepeticoes, r);
            if (referencia >= 0 && r.melhorValor != referencia) {
                throw std::logic_error("estratégias divergiram em " + p.experimento);
            }
            referencia = r.melhorValor;
            csv << p.experimento << ',' << e.nome << ',' << p.n << ',' << p.capacidade << ',' << ms << ','
                << r.operacoes << ',' << e.operacoesTeoricas(p.n, p.capacidade) << ',' << r.melhorValor << '\n';
            std::cout << p.experimento << " n=" << p.n << " C=" << p.capacidade << ' ' << e.nome
                      << ": " << ms << " ms, " << r.operacoes << " ops\n";
        }
    }
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc == 3 && std::string(argv[1]) == "--benchmark") return rodarBenchmark(argv[2]);
        if (argc == 2) return resolverArquivo(argv[1]);
        std::cerr << "uso: " << argv[0] << " <instancia.txt> | --benchmark <saida.csv>\n";
        return 2;
    } catch (const std::exception& erro) {
        std::cerr << "erro: " << erro.what() << '\n';
        return 1;
    }
}
