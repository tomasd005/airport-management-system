#include "gestor_interativo.h"
#include "gestores/gestor_aeroportos.h"
#include "gestores/gestor_avioes.h"
#include "gestores/gestor_voos.h"
#include "gestores/gestor_passageiros.h"
#include "gestores/gestor_reservas.h"
#include "queries/querie1.h"
#include "queries/querie2.h"
#include "queries/querie3.h"
#include "queries/querie4.h"
#include "queries/querie5.h"
#include "queries/querie6.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 512
#define DEFAULT_DATASET "./dataset"

/* Códigos ANSI para cores no terminal */
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"

/**
 * @struct GestorInterativo
 * @brief Estrutura do gestor do modo interativo.
 *
 * Contém os gestores de dados necessários à execução das queries
 * e um indicador que sinaliza se os dados já foram carregados.
 */
struct GestorInterativo {
    gestor_aeroportos_t *aeroportos;
    gestor_avioes_t *avioes;
    gestor_voos_t *voos;
    gestor_passageiros_t *passageiros;
    gestor_reservas_t *reservas;
    int dados_carregados;
};

/**
 * @brief Cria um novo gestor para o modo interativo.
 *
 * Inicializa todos os gestores de dados necessários.
 *
 * @return Ponteiro para o gestor interativo criado ou NULL em erro
 */
gestor_interativo_t *gestor_interativo_criar(void)
{
    gestor_interativo_t *gestor = malloc(sizeof(gestor_interativo_t));
    if (!gestor)
        return NULL;

    gestor->aeroportos = gestor_aeroportos_criar();
    gestor->avioes = gestor_avioes_criar();
    gestor->voos = gestor_voos_criar();
    gestor->passageiros = gestor_passageiros_criar();
    gestor->reservas = gestor_reservas_criar();
    gestor->dados_carregados = 0;

    return gestor;
}

/**
 * @brief Destrói o gestor interativo e liberta a memória associada.
 *
 * @param gestor Ponteiro para o gestor interativo
 */
void gestor_interativo_destruir(gestor_interativo_t *gestor)
{
    if (!gestor)
        return;

    gestor_aeroportos_destruir(gestor->aeroportos);
    gestor_avioes_destruir(gestor->avioes);
    gestor_voos_destruir(gestor->voos);
    gestor_passageiros_destruir(gestor->passageiros);
    gestor_reservas_destruir(gestor->reservas);

    free(gestor);
}

/**
 * @brief Remove espaços e quebras de linha de uma string.
 *
 * @param str String a limpar
 */
static void limpar_input(char *str)
{
    if (!str)
        return;

    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }

    char *end = str + strlen(str) - 1;
    while (end >= str && isspace(*end)) {
        *end = '\0';
        end--;
    }

    char *start = str;
    while (*start && isspace(*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);
}

static void copiar_dataset_padrao(char *dest, size_t dest_size)
{
    if (!dest || dest_size == 0)
        return;
    snprintf(dest, dest_size, "%s", DEFAULT_DATASET);
}

/**
 * @brief Carrega todos os ficheiros CSV do dataset.
 *
 * @param gestor Gestor interativo
 * @param pasta Caminho para a pasta do dataset
 */
static void carregar_dataset(gestor_interativo_t *gestor, const char *pasta)
{
    printf(CYAN "Carregando dataset de: %s\n" RESET, pasta);
    printf("Isso pode demorar alguns segundos...\n\n");

    char caminho[BUFFER_SIZE];

    printf("  [1/5] Carregando aeroportos...\n");
    snprintf(caminho, sizeof(caminho), "%s/airports.csv", pasta);
    gestor_aeroportos_carregar(gestor->aeroportos, caminho);

    printf("  [2/5] Carregando aviões...\n");
    snprintf(caminho, sizeof(caminho), "%s/aircrafts.csv", pasta);
    gestor_avioes_carregar(gestor->avioes, caminho);

    printf("  [3/5] Carregando voos...\n");
    snprintf(caminho, sizeof(caminho), "%s/flights.csv", pasta);
    gestor_voos_carregar_com_validacao(gestor->voos, caminho, gestor->avioes);

    printf("  [4/5] Carregando passageiros...\n");
    snprintf(caminho, sizeof(caminho), "%s/passengers.csv", pasta);
    gestor_passageiros_carregar(gestor->passageiros, caminho);

    printf("  [5/5] Carregando reservas...\n");
    snprintf(caminho, sizeof(caminho), "%s/reservations.csv", pasta);
    gestor_reservas_carregar_com_validacao(gestor->reservas, caminho, gestor->voos,
                                           gestor->passageiros);

    gestor_reservas_finalizar(gestor->reservas);
    gestor_voos_atualizar_contagens_aeroportos(gestor->voos, gestor->aeroportos);
    gestor_voos_preparar_q3(gestor->voos);

    gestor->dados_carregados = 1;

    printf(GREEN "\n✓ Dataset carregado com sucesso!\n" RESET);
    printf("   Aeroportos: %u\n", gestor_aeroportos_contar(gestor->aeroportos));
    printf("   Aviões: %u\n", gestor_avioes_contar(gestor->avioes));
    printf("   Voos: %u\n", gestor_voos_contar(gestor->voos));
    printf("   Passageiros: %u\n", gestor_passageiros_numero(gestor->passageiros));
    printf("   Reservas: %u\n\n", gestor_reservas_numero(gestor->reservas));
}

/**
 * @brief Apresenta o menu principal do modo interativo.
 */
static void mostrar_menu(void)
{
    printf(BOLD BLUE "\n╔═══════════════════════════════════════════════╗\n" RESET);
    printf(BOLD BLUE "║" RESET "      SISTEMA DE GESTÃO DE VOOS - LI3      " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "╠═══════════════════════════════════════════════╣\n" RESET);
    printf(BOLD BLUE "║" RESET " " BOLD "Queries Disponíveis:" RESET
                     "                        " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET "                                            " BOLD BLUE
                     "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "1" RESET
                     " - Resumo de aeroporto                    " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "2" RESET
                     " - Top N aviões com mais voos             " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "3" RESET
                     " - Aeroporto com mais partidas (período)  " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "4" RESET
                     " - Passageiro no top 10 mais vezes        " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "5" RESET
                     " - Companhias com mais atrasos            " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "6" RESET
                     " - Destino mais comum (nacionalidade)     " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET "                                            " BOLD BLUE
                     "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " YELLOW "0" RESET
                     " - Sair                                   " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "╚═══════════════════════════════════════════════╝\n" RESET);
}

static void executar_query1(gestor_interativo_t *gestor)
{
    char codigo[10];
    printf(CYAN "\n→ Query 1: Resumo de Aeroporto\n" RESET);
    printf("Código do aeroporto (ex: OPO): ");

    if (!fgets(codigo, sizeof(codigo), stdin))
        return;
    limpar_input(codigo);

    if (strlen(codigo) == 0) {
        printf(RED "✗ Código inválido!\n" RESET);
        return;
    }

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[32];
    snprintf(comando, sizeof(comando), "1 %s", codigo);
    query1(gestor->aeroportos, gestor->voos, gestor->reservas, comando, codigo, stdout);
}

static void executar_query2(gestor_interativo_t *gestor)
{
    char input_n[20], fabricante[100];

    printf(CYAN "\n→ Query 2: Top N Aviões com Mais Voos\n" RESET);
    printf("Número de aviões (N): ");

    if (!fgets(input_n, sizeof(input_n), stdin))
        return;
    limpar_input(input_n);

    int N = atoi(input_n);
    if (N <= 0) {
        printf(RED "✗ Número inválido!\n" RESET);
        return;
    }

    printf("Fabricante (opcional, Enter para todos): ");
    if (!fgets(fabricante, sizeof(fabricante), stdin))
        return;
    limpar_input(fabricante);

    char *fab = (strlen(fabricante) > 0) ? fabricante : NULL;

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[128];
    snprintf(comando, sizeof(comando), "2 %d %s", N, fab ? fab : "");
    query2(gestor->avioes, gestor->voos, N, fab, comando, stdout);
}

static void executar_query3(gestor_interativo_t *gestor)
{
    char data_inicio[20], data_fim[20];

    printf(CYAN "\n→ Query 3: Aeroporto com Mais Partidas (Período)\n" RESET);
    printf("Data inicial (YYYY-MM-DD): ");

    if (!fgets(data_inicio, sizeof(data_inicio), stdin))
        return;
    limpar_input(data_inicio);

    printf("Data final (YYYY-MM-DD): ");
    if (!fgets(data_fim, sizeof(data_fim), stdin))
        return;
    limpar_input(data_fim);

    if (strlen(data_inicio) < 10 || strlen(data_fim) < 10) {
        printf(RED "✗ Datas inválidas!\n" RESET);
        return;
    }

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[64];
    snprintf(comando, sizeof(comando), "3 %s %s", data_inicio, data_fim);
    query3(gestor->aeroportos, gestor->voos, data_inicio, data_fim, comando, stdout);
}

static void executar_query4(gestor_interativo_t *gestor)
{
    char resposta[10], data_inicio[20], data_fim[20];

    printf(CYAN "\n→ Query 4: Passageiro no Top 10 Mais Vezes\n" RESET);
    printf("Filtrar por período? (s/n): ");

    if (!fgets(resposta, sizeof(resposta), stdin))
        return;
    limpar_input(resposta);

    char *di = NULL, *df = NULL;

    if (resposta[0] == 's' || resposta[0] == 'S') {
        printf("Data inicial (YYYY-MM-DD): ");
        if (!fgets(data_inicio, sizeof(data_inicio), stdin))
            return;
        limpar_input(data_inicio);

        printf("Data final (YYYY-MM-DD): ");
        if (!fgets(data_fim, sizeof(data_fim), stdin))
            return;
        limpar_input(data_fim);

        di = data_inicio;
        df = data_fim;
    }

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[64];
    if (di && df)
        snprintf(comando, sizeof(comando), "4 %s %s", di, df);
    else
        snprintf(comando, sizeof(comando), "4");

    query4(gestor->reservas, gestor->voos, gestor->passageiros, di, df, comando, stdout);
}

static void executar_query5(gestor_interativo_t *gestor)
{
    char input_n[20];

    printf(CYAN "\n→ Query 5: Companhias com Mais Atrasos\n" RESET);
    printf("Número de companhias (N): ");

    if (!fgets(input_n, sizeof(input_n), stdin))
        return;
    limpar_input(input_n);

    int N = atoi(input_n);
    if (N <= 0) {
        printf(RED "✗ Número inválido!\n" RESET);
        return;
    }

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[32];
    snprintf(comando, sizeof(comando), "5 %d", N);
    query5(gestor->voos, N, comando, stdout);
}

static void executar_query6(gestor_interativo_t *gestor)
{
    char nacionalidade[100];

    printf(CYAN "\n→ Query 6: Destino Mais Comum (Nacionalidade)\n" RESET);
    printf("Nacionalidade: ");

    if (!fgets(nacionalidade, sizeof(nacionalidade), stdin))
        return;
    limpar_input(nacionalidade);

    if (strlen(nacionalidade) == 0) {
        printf(RED "✗ Nacionalidade inválida!\n" RESET);
        return;
    }

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[128];
    snprintf(comando, sizeof(comando), "6 %s", nacionalidade);
    query6(gestor->reservas, gestor->voos, gestor->passageiros, nacionalidade, comando, stdout);
}

/**
 * @brief Executa o ciclo principal do modo interativo.
 *
 * Solicita o caminho do dataset, carrega os dados e apresenta
 * o menu de queries até o utilizador escolher sair.
 *
 * @param gestor Gestor interativo
 */
void gestor_interativo_executar(gestor_interativo_t *gestor)
{
    char caminho_dataset[BUFFER_SIZE];
    char opcao[10];

    // Banner
    printf(BOLD GREEN "\n");
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║                                                       ║\n");
    printf("║         SISTEMA DE GESTÃO DE VOOS - LI3 2025         ║\n");
    printf("║              Modo Interativo                          ║\n");
    printf("║                                                       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf(RESET "\n");

    // Solicitar caminho
    printf("Introduza o caminho dos ficheiros de dados\n");
    printf("(deixe vazio para usar '%s'): ", DEFAULT_DATASET);

    if (fgets(caminho_dataset, sizeof(caminho_dataset), stdin)) {
        limpar_input(caminho_dataset);

        if (strlen(caminho_dataset) == 0) {
            copiar_dataset_padrao(caminho_dataset, sizeof(caminho_dataset));
        }
    } else {
        copiar_dataset_padrao(caminho_dataset, sizeof(caminho_dataset));
    }

    // Carregar dataset
    carregar_dataset(gestor, caminho_dataset);

    // Loop principal
    while (1) {
        mostrar_menu();
        printf(BOLD "Escolha uma opção: " RESET);

        if (!fgets(opcao, sizeof(opcao), stdin))
            break;
        limpar_input(opcao);

        if (strlen(opcao) == 0)
            continue;

        int escolha = atoi(opcao);

        printf("\n");

        switch (escolha) {
        case 0:
            printf(YELLOW "Encerrando programa...\n" RESET);
            printf(GREEN "\n✓ Programa encerrado. Até breve!\n" RESET);
            return;

        case 1:
            executar_query1(gestor);
            break;

        case 2:
            executar_query2(gestor);
            break;

        case 3:
            executar_query3(gestor);
            break;

        case 4:
            executar_query4(gestor);
            break;

        case 5:
            executar_query5(gestor);
            break;

        case 6:
            executar_query6(gestor);
            break;

        default:
            printf(RED "✗ Opção inválida! Escolha entre 0-6.\n" RESET);
            break;
        }

        printf("\n" YELLOW "Pressione Enter para continuar..." RESET);
        getchar();
    }
}
