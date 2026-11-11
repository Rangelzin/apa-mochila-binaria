/**
 * @file main.cpp
 * @brief Driver: resolve uma instância de arquivo, roda o benchmark (tempo + operações),
 *        o experimento de qualidade das heurísticas ou o das estratégias exponenciais.
 *
 * Uso:
 *   ./build/mochila data/exemplo.txt                      resolve e imprime resultado de cada estratégia
 *   ./build/mochila --benchmark results/resultados.csv    executa os experimentos de tempo e grava CSV
 *   ./build/mochila --qualidade results/qualidade.csv     mede valor heurístico / ótimo e grava CSV
 *   ./build/mochila --exponencial results/exponencial.csv mede força bruta e backtracking em n pequeno
 */
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "mochila.hpp"

namespace {

using mochila::Contador;
using mochila::Instancia;
using mochila::Resultado;
using mochila::Valor;

struct Estrategia {
    std::string nome;
    std::function<Resultado(const Instancia&)> resolver;
    std::function<Contador(int n, int capacidade)> operacoesTeoricas;  // 0 = depende dos dados
    bool exata;              // true: sempre devolve o ótimo
    double garantia = 0.0;   // heurísticas: razão mínima garantida com o ótimo (0 = nenhuma)
    bool exponencial = false;  // true: custo 2^n, só entra no experimento --exponencial
    std::size_t maxN = 0;      // 0 = sem limite; acima disso a estratégia é pulada
};

/**
 * @brief Tetos de n das estratégias exponenciais.
 * A força bruta percorre 2^n sempre, então o teto é o do tempo de execução. O backtracking
 * poda e na prática vai muito além, mas a poda não dá garantia: o teto é precaução contra o
 * pior caso, não uma propriedade do algoritmo.
 */
constexpr std::size_t kMaxForcaBruta = 24;
constexpr std::size_t kMaxBacktracking = 40;

/** @brief A estratégia pode ser executada com esse n? */
bool viavel(const Estrategia& e, std::size_t n) { return e.maxN == 0 || n <= e.maxN; }

std::vector<Estrategia> estrategias() {
    const auto dependeDosDados = [](int, int) -> Contador { return 0; };
    return {
        {"forca_bruta", mochila::resolverForcaBruta,
         [](int n, int) -> Contador {
             if (n < 0 || static_cast<std::size_t>(n) > kMaxForcaBruta) return 0;  // 2^n não cabe
             return static_cast<Contador>(n) * (Contador{1} << n);
         },
         true, 0.0, true, kMaxForcaBruta},
        {"backtracking", mochila::resolverBacktracking, dependeDosDados, true, 0.0, true,
         kMaxBacktracking},
        {"divisao_conquista", mochila::resolverDivisaoConquista,
         [](int n, int c) -> Contador {
             if (n == 0) return 0;
             const Contador cc = static_cast<Contador>(c);
             return static_cast<Contador>(n) * (cc + 1) +
                    static_cast<Contador>(n - 1) * (cc + 1) * (cc + 2) / 2;
         },
         true},
        {"pd_tabela", mochila::resolverProgramacaoDinamica,
         [](int n, int c) -> Contador { return static_cast<Contador>(n) * (static_cast<Contador>(c) + 1); },
         true},
        {"pd_vetor", mochila::resolverProgramacaoDinamicaVetor, dependeDosDados, true},
        {"guloso", mochila::resolverGuloso, dependeDosDados, false, 0.0},
        {"aproximado", mochila::resolverAproximado, dependeDosDados, false, 0.5},
    };
}

/** @brief Certificado viável (índices distintos, cabe em C) que soma exatamente melhorValor? */
bool solucaoConsistente(const Instancia& instancia, const Resultado& r) {
    Instancia comoAlvo = instancia;
    comoAlvo.alvo = r.melhorValor;
    Valor soma = 0;
    for (int i : r.selecionados) soma += instancia.itens[mochila::paraIndice(i)].valor;
    return soma == r.melhorValor && mochila::verificarCertificado(comoAlvo, r.selecionados);
}

/**
 * @brief Confere uma heurística contra o ótimo: solução viável, valor ≤ ótimo e
 * respeito à garantia de aproximação declarada.
 */
void conferirHeuristica(const Estrategia& e, const Instancia& instancia, const Resultado& r, Valor otimo,
                        const std::string& contexto) {
    if (!solucaoConsistente(instancia, r)) {
        throw std::logic_error(e.nome + " devolveu solução inconsistente em " + contexto);
    }
    if (r.melhorValor > otimo) {
        throw std::logic_error(e.nome + " superou o ótimo em " + contexto);
    }
    if (static_cast<double>(r.melhorValor) < e.garantia * static_cast<double>(otimo)) {
        throw std::logic_error(e.nome + " violou a garantia de aproximação em " + contexto);
    }
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

void imprimirResultado(const Estrategia& e, const Resultado& r, const Instancia& instancia) {
    // Uma heurística que não atinge o alvo não prova que a resposta é NÃO, a menos que tenha
    // garantia g: OPT ≤ valor/g, então valor < g·V implica OPT < V.
    const char* resposta = "SIM";
    if (!r.atingeAlvo) {
        if (e.exata) {
            resposta = "NAO";
        } else if (static_cast<double>(r.melhorValor) < e.garantia * static_cast<double>(instancia.alvo)) {
            resposta = "NAO (pela garantia de aproximacao)";
        } else {
            resposta = "NAO SEI (heuristica)";
        }
    }
    std::cout << e.nome << ": melhor valor = " << r.melhorValor
              << " | resposta (>= " << instancia.alvo << ") = " << resposta
              << " | operacoes = " << r.operacoes;
    if (!r.selecionados.empty()) {
        std::cout << " | itens = {";
        for (std::size_t k = 0; k < r.selecionados.size(); ++k) {
            std::cout << (k ? ", " : "") << r.selecionados[k] + 1;  // 1-based na saída
        }
        std::cout << '}';
        if (r.atingeAlvo) {
            std::cout << " | certificado valido = "
                      << (mochila::verificarCertificado(instancia, r.selecionados) ? "sim" : "nao");
        } else {
            std::cout << " | solucao viavel = " << (solucaoConsistente(instancia, r) ? "sim" : "nao");
        }
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
        if (!viavel(e, instancia.itens.size())) {
            std::cout << e.nome << ": pulado (n > " << e.maxN << ", 2^n inviavel)\n";
            continue;
        }
        imprimirResultado(e, e.resolver(instancia), instancia);
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

/** @brief Pontos do experimento D: n pequeno, a faixa em que 2^n ainda é viável. */
std::vector<Ponto> pontosExponenciais() {
    std::vector<Ponto> pontos;
    for (int n : {4, 8, 12, 16, 20, 22, 24, 28, 32, 36, 40}) {
        pontos.push_back({"exponencial", n, 1000});
    }
    return pontos;
}

/**
 * @brief Mede cada estratégia em cada ponto, confere o resultado contra o ótimo e grava uma
 * linha de CSV por medição. As exponenciais só entram quando `incluirExponenciais`.
 */
void medirPontos(const std::vector<Ponto>& pontos, std::ostream& csv, int repeticoes,
                 bool incluirExponenciais) {
    constexpr unsigned kSemente = 42;
    for (const Ponto& p : pontos) {
        const Instancia instancia = mochila::gerarInstancia(p.n, p.capacidade, kSemente);
        const std::string contexto = p.experimento + " n=" + std::to_string(p.n) + " C=" + std::to_string(p.capacidade);
        const Valor otimo = mochila::resolverProgramacaoDinamicaVetor(instancia).melhorValor;
        for (const Estrategia& e : estrategias()) {
            if (e.exponencial && !incluirExponenciais) continue;  // 2^n: ver --exponencial
            if (!viavel(e, instancia.itens.size())) continue;
            Resultado r;
            const double ms = medirMedianaMs(e, instancia, repeticoes, r);
            if (e.exata && r.melhorValor != otimo) {
                throw std::logic_error("estratégias exatas divergiram em " + contexto);
            }
            if (!e.exata) conferirHeuristica(e, instancia, r, otimo, contexto);
            const double razao = otimo > 0 ? static_cast<double>(r.melhorValor) / static_cast<double>(otimo) : 1.0;
            csv << p.experimento << ',' << e.nome << ',' << p.n << ',' << p.capacidade << ',' << ms << ','
                << r.operacoes << ',' << e.operacoesTeoricas(p.n, p.capacidade) << ',' << r.melhorValor << ','
                << razao << '\n';
            std::cout << contexto << ' ' << e.nome << ": " << ms << " ms, " << r.operacoes << " ops\n";
        }
    }
}

/** @brief Abre o CSV de saída já com o cabeçalho das medições de tempo. */
std::ofstream abrirCsvDeTempo(const std::string& caminho) {
    std::ofstream csv(caminho);
    if (!csv) throw std::runtime_error("não foi possível criar " + caminho);
    csv << "experimento,estrategia,n,C,tempo_ms,operacoes,operacoes_teoricas,melhor_valor,razao_otimo\n";
    return csv;
}

int rodarBenchmark(const std::string& caminhoCsv) {
    std::ofstream csv = abrirCsvDeTempo(caminhoCsv);
    medirPontos(pontosDoExperimento(), csv, 5, false);
    return 0;
}

/**
 * @brief Experimento D: compara as estratégias exponenciais com as demais na faixa de n em
 * que a enumeração completa ainda termina. Menos repetições, porque cada execução é cara.
 */
int rodarExponencial(const std::string& caminhoCsv) {
    std::ofstream csv = abrirCsvDeTempo(caminhoCsv);
    medirPontos(pontosExponenciais(), csv, 3, true);
    return 0;
}

/**
 * @brief Instância fortemente correlacionada (v_i = p_i + C/10), família clássica em que a
 * densidade quase não distingue os itens e o guloso tende a desperdiçar capacidade.
 */
Instancia gerarInstanciaCorrelacionada(int n, int capacidade, unsigned semente) {
    std::mt19937 gerador(semente);
    std::uniform_int_distribution<int> distPeso(1, capacidade > 0 ? capacidade : 1);
    Instancia instancia{{}, capacidade, 0};
    Valor soma = 0;
    for (int i = 0; i < n; ++i) {
        const int peso = distPeso(gerador);
        const Valor valor = peso + capacidade / 10;
        soma += valor;
        instancia.itens.push_back({peso, valor});
    }
    instancia.alvo = soma / 2;
    return instancia;
}

/**
 * @brief Para cada família e cada n, resolve kInstancias instâncias e registra a razão
 * valor/ótimo das heurísticas (mínima, média) e a fração de instâncias resolvidas no ótimo.
 */
int rodarQualidade(const std::string& caminhoCsv) {
    constexpr int kInstancias = 200;
    constexpr int kCapacidade = 1000;
    std::ofstream csv(caminhoCsv);
    if (!csv) throw std::runtime_error("não foi possível criar " + caminhoCsv);
    csv << "familia,estrategia,n,C,instancias,razao_min,razao_media,frac_otimo\n";

    struct Familia {
        std::string nome;
        std::function<Instancia(int, int, unsigned)> gerar;
    };
    const std::vector<Familia> familias = {
        {"aleatoria", mochila::gerarInstancia},
        {"correlacionada", gerarInstanciaCorrelacionada},
    };

    std::vector<Estrategia> heuristicas;
    for (const Estrategia& e : estrategias()) {
        if (!e.exata) heuristicas.push_back(e);
    }

    for (const Familia& f : familias) {
        for (int n : {5, 10, 20, 50, 100, 200, 500, 1000}) {
            std::vector<double> razaoMin(heuristicas.size(), 1.0);
            std::vector<double> somaRazao(heuristicas.size(), 0.0);
            std::vector<int> otimos(heuristicas.size(), 0);
            for (int s = 0; s < kInstancias; ++s) {
                const Instancia instancia = f.gerar(n, kCapacidade, static_cast<unsigned>(s));
                const Valor otimo = mochila::resolverProgramacaoDinamicaVetor(instancia).melhorValor;
                const std::string contexto = f.nome + " n=" + std::to_string(n) + " semente=" + std::to_string(s);
                for (std::size_t h = 0; h < heuristicas.size(); ++h) {
                    const Resultado r = heuristicas[h].resolver(instancia);
                    conferirHeuristica(heuristicas[h], instancia, r, otimo, contexto);
                    const double razao =
                        otimo > 0 ? static_cast<double>(r.melhorValor) / static_cast<double>(otimo) : 1.0;
                    razaoMin[h] = std::min(razaoMin[h], razao);
                    somaRazao[h] += razao;
                    if (r.melhorValor == otimo) ++otimos[h];
                }
            }
            for (std::size_t h = 0; h < heuristicas.size(); ++h) {
                const double media = somaRazao[h] / kInstancias;
                const double fracOtimo = static_cast<double>(otimos[h]) / kInstancias;
                csv << f.nome << ',' << heuristicas[h].nome << ',' << n << ',' << kCapacidade << ','
                    << kInstancias << ',' << razaoMin[h] << ',' << media << ',' << fracOtimo << '\n';
                std::cout << f.nome << " n=" << n << ' ' << heuristicas[h].nome << ": min " << razaoMin[h]
                          << ", media " << media << ", otimo em " << fracOtimo * 100 << "%\n";
            }
        }
    }
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc == 3 && std::string(argv[1]) == "--benchmark") return rodarBenchmark(argv[2]);
        if (argc == 3 && std::string(argv[1]) == "--qualidade") return rodarQualidade(argv[2]);
        if (argc == 3 && std::string(argv[1]) == "--exponencial") return rodarExponencial(argv[2]);
        if (argc == 2) return resolverArquivo(argv[1]);
        std::cerr << "uso: " << argv[0]
                  << " <instancia.txt> | --benchmark <saida.csv> | --qualidade <saida.csv>"
                     " | --exponencial <saida.csv>\n";
        return 2;
    } catch (const std::exception& erro) {
        std::cerr << "erro: " << erro.what() << '\n';
        return 1;
    }
}
