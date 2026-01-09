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
struct GestorInterativo
{
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
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r'))
        str[--len] = '\0';

    char *end = str + strlen(str) - 1;
    while (end >= str && isspace(*end))
        *end-- = '\0';

    char *start = str;
    while (*start && isspace(*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);
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

    snprintf(caminho, sizeof(caminho), "%s/airports.csv", pasta);
    gestor_aeroportos_carregar(gestor->aeroportos, caminho);

    snprintf(caminho, sizeof(caminho), "%s/aircrafts.csv", pasta);
    gestor_avioes_carregar(gestor->avioes, caminho);

    snprintf(caminho, sizeof(caminho), "%s/flights.csv", pasta);
    gestor_voos_carregar_com_validacao(gestor->voos, caminho, gestor->avioes);

    snprintf(caminho, sizeof(caminho), "%s/passengers.csv", pasta);
    gestor_passageiros_carregar(gestor->passageiros, caminho);

    snprintf(caminho, sizeof(caminho), "%s/reservations.csv", pasta);
    gestor_reservas_carregar_com_validacao(
        gestor->reservas, caminho,
        gestor->voos, gestor->passageiros);

    gestor_reservas_finalizar(gestor->reservas);
    gestor_voos_atualizar_contagens_aeroportos(gestor->voos, gestor->aeroportos);
    gestor_voos_preparar_q3(gestor->voos);

    gestor->dados_carregados = 1;

    printf(GREEN "\n✓ Dataset carregado com sucesso!\n" RESET);
}

/**
 * @brief Apresenta o menu principal do modo interativo.
 */
static void mostrar_menu(void)
{
    printf(BOLD BLUE "\n╔═══════════════════════════════════════════════╗\n" RESET);
    printf(BOLD BLUE "║" RESET "      SISTEMA DE GESTÃO DE VOOS - LI3      " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "╠═══════════════════════════════════════════════╣\n" RESET);
    printf(BOLD BLUE "║" RESET " Queries Disponíveis                         " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " 1 - Resumo de aeroporto                     " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " 2 - Top N aviões com mais voos              " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " 3 - Aeroporto com mais partidas (período)   " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " 4 - Passageiro no top 10 mais vezes         " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " 5 - Companhias com mais atrasos             " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " 6 - Destino mais comum (nacionalidade)      " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " 0 - Sair                                    " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "╚═══════════════════════════════════════════════╝\n" RESET);
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

    printf(BOLD GREEN "\nSISTEMA DE GESTÃO DE VOOS - LI3 2025\n" RESET);
    printf("Modo Interativo\n\n");

    printf("Introduza o caminho dos ficheiros de dados\n");
    printf("(Enter para usar '%s'): ", DEFAULT_DATASET);

    if (fgets(caminho_dataset, sizeof(caminho_dataset), stdin))
    {
        limpar_input(caminho_dataset);
        if (strlen(caminho_dataset) == 0)
            strcpy(caminho_dataset, DEFAULT_DATASET);
    }
    else
    {
        strcpy(caminho_dataset, DEFAULT_DATASET);
    }

    carregar_dataset(gestor, caminho_dataset);

    while (1)
    {
        mostrar_menu();
        printf(BOLD "Escolha uma opção: " RESET);

        if (!fgets(opcao, sizeof(opcao), stdin))
            break;
        limpar_input(opcao);

        int escolha = atoi(opcao);

        switch (escolha)
        {
        case 0:
            printf(GREEN "\n✓ Programa encerrado. Até breve!\n" RESET);
            return;
        case 1: executar_query1(gestor); break;
        case 2: executar_query2(gestor); break;
        case 3: executar_query3(gestor); break;
        case 4: executar_query4(gestor); break;
        case 5: executar_query5(gestor); break;
        case 6: executar_query6(gestor); break;
        default:
            printf(RED "✗ Opção inválida!\n" RESET);
            break;
        }

        printf(YELLOW "\nPressione Enter para continuar..." RESET);
        getchar();
    }
}
