#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/gestores/gestor_aeroportos.h"
#include "../../include/gestores/gestor_avioes.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/gestores/gestor_passageiros.h"
#include "../../include/gestores/gestor_reservas.h"
#include "../../include/queries/querie1.h"
#include "../../include/queries/querie2.h"
#include "../../include/queries/querie3.h"
#include "../../include/queries/querie4.h"
#include "../../include/queries/querie5.h"
#include "../../include/queries/querie6.h"

#define BUFFER_SIZE 512
#define DEFAULT_DATASET "./dataset"

// Estrutura para armazenar os gestores
typedef struct
{
    gestor_aeroportos_t *aeroportos;
    gestor_avioes_t *avioes;
    gestor_voos_t *voos;
    gestor_passageiros_t *passageiros;
    gestor_reservas_t *reservas;
    int carregado;
} Sistema;

// Cores ANSI para terminal
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

static void limpar_input(char *str)
{
    if (!str)
        return;

    // Remove newline
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r'))
    {
        str[len - 1] = '\0';
        len--;
    }

    // Trim espaços
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace(*end))
    {
        *end = '\0';
        end--;
    }

    char *start = str;
    while (*start && isspace(*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);
}

static void carregar_dataset(Sistema *sys, const char *pasta)
{
    printf(CYAN "Carregando dataset de: %s\n" RESET, pasta);
    printf("Isso pode demorar alguns segundos...\n\n");

    char caminho[BUFFER_SIZE];

    // Aeroportos
    printf("  [1/5] Carregando aeroportos...\n");
    snprintf(caminho, sizeof(caminho), "%s/airports.csv", pasta);
    gestor_aeroportos_carregar(sys->aeroportos, caminho);

    // Aviões
    printf("  [2/5] Carregando aviões...\n");
    snprintf(caminho, sizeof(caminho), "%s/aircrafts.csv", pasta);
    gestor_avioes_carregar(sys->avioes, caminho);

    // Voos
    printf("  [3/5] Carregando voos...\n");
    snprintf(caminho, sizeof(caminho), "%s/flights.csv", pasta);
    gestor_voos_carregar(sys->voos, caminho);

    // Passageiros
    printf("  [4/5] Carregando passageiros...\n");
    snprintf(caminho, sizeof(caminho), "%s/passengers.csv", pasta);
    gestor_passageiros_carregar(sys->passageiros, caminho);

    // Reservas
    printf("  [5/5] Carregando reservas...\n");
    snprintf(caminho, sizeof(caminho), "%s/reservations.csv", pasta);
    gestor_reservas_carregar(sys->reservas, caminho);

    sys->carregado = 1;

    printf(GREEN "\n✓ Dataset carregado com sucesso!\n" RESET);
    printf("  • Aeroportos: %u\n", gestor_aeroportos_contar(sys->aeroportos));
    printf("  • Aviões: %u\n", gestor_avioes_contar(sys->avioes));
    printf("  • Voos: %u\n", gestor_voos_contar(sys->voos));
    printf("  • Passageiros: %u\n", gestor_passageiros_numero(sys->passageiros));
    printf("  • Reservas: %u\n\n", gestor_reservas_numero(sys->reservas));
}

static void mostrar_menu()
{
    printf(BOLD BLUE "\n╔══════════════════════════════════════════════╗\n" RESET);
    printf(BOLD BLUE "║" RESET "      SISTEMA DE GESTÃO DE VOOS - LI3      " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "╠══════════════════════════════════════════════╣\n" RESET);
    printf(BOLD BLUE "║" RESET " " BOLD "Queries Disponíveis:" RESET "                        " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET "                                            " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "1" RESET " - Resumo de aeroporto                    " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "2" RESET " - Top N aviões com mais voos             " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "3" RESET " - Aeroporto com mais partidas (período)  " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "4" RESET " - Passageiro no top 10 mais vezes        " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "5" RESET " - Companhias com mais atrasos            " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "6" RESET " - Destino mais comum (nacionalidade)     " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET "                                            " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " YELLOW "0" RESET " - Sair                                   " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "╚══════════════════════════════════════════════╝\n" RESET);
}

static void executar_query1(Sistema *sys)
{
    char codigo[10];
    printf(CYAN "\n→ Query 1: Resumo de Aeroporto\n" RESET);
    printf("Código do aeroporto (ex: OPO): ");

    if (!fgets(codigo, sizeof(codigo), stdin))
        return;
    limpar_input(codigo);

    if (strlen(codigo) == 0)
    {
        printf(RED "✗ Código inválido!\n" RESET);
        return;
    }

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[32];
    snprintf(comando, sizeof(comando), "1 %s", codigo);
    query1(sys->aeroportos, sys->voos, sys->reservas, comando, codigo, stdout);
}

static void executar_query2(Sistema *sys)
{
    char input_n[20], fabricante[100];

    printf(CYAN "\n→ Query 2: Top N Aviões com Mais Voos\n" RESET);
    printf("Número de aviões (N): ");

    if (!fgets(input_n, sizeof(input_n), stdin))
        return;
    limpar_input(input_n);

    int N = atoi(input_n);
    if (N <= 0)
    {
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
    query2(sys->avioes, sys->voos, N, fab, comando, stdout);
}

static void executar_query3(Sistema *sys)
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

    if (strlen(data_inicio) < 10 || strlen(data_fim) < 10)
    {
        printf(RED "✗ Datas inválidas!\n" RESET);
        return;
    }

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[64];
    snprintf(comando, sizeof(comando), "3 %s %s", data_inicio, data_fim);
    query3(sys->aeroportos, sys->voos, data_inicio, data_fim, comando, stdout);
}

static void executar_query4(Sistema *sys)
{
    char resposta[10], data_inicio[20], data_fim[20];

    printf(CYAN "\n→ Query 4: Passageiro no Top 10 Mais Vezes\n" RESET);
    printf("Filtrar por período? (s/n): ");

    if (!fgets(resposta, sizeof(resposta), stdin))
        return;
    limpar_input(resposta);

    char *di = NULL, *df = NULL;

    if (resposta[0] == 's' || resposta[0] == 'S')
    {
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

    query4(sys->reservas, sys->voos, sys->passageiros, di, df, comando, stdout);
}

static void executar_query5(Sistema *sys)
{
    char input_n[20];

    printf(CYAN "\n→ Query 5: Companhias com Mais Atrasos\n" RESET);
    printf("Número de companhias (N): ");

    if (!fgets(input_n, sizeof(input_n), stdin))
        return;
    limpar_input(input_n);

    int N = atoi(input_n);
    if (N <= 0)
    {
        printf(RED "✗ Número inválido!\n" RESET);
        return;
    }

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[32];
    snprintf(comando, sizeof(comando), "5 %d", N);
    query5(sys->voos, N, comando, stdout);
}

static void executar_query6(Sistema *sys)
{
    char nacionalidade[100];

    printf(CYAN "\n→ Query 6: Destino Mais Comum (Nacionalidade)\n" RESET);
    printf("Nacionalidade: ");

    if (!fgets(nacionalidade, sizeof(nacionalidade), stdin))
        return;
    limpar_input(nacionalidade);

    if (strlen(nacionalidade) == 0)
    {
        printf(RED "✗ Nacionalidade inválida!\n" RESET);
        return;
    }

    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[128];
    snprintf(comando, sizeof(comando), "6 %s", nacionalidade);
    query6(sys->reservas, sys->voos, sys->passageiros,
           nacionalidade, comando, stdout);
}

int main(void)
{
    Sistema sys = {0};
    char caminho_dataset[BUFFER_SIZE];
    char opcao[10];

    // Banner inicial
    printf(BOLD GREEN "\n");
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║                                                       ║\n");
    printf("║         SISTEMA DE GESTÃO DE VOOS - LI3 2025         ║\n");
    printf("║              Modo Interativo - Fase 2                ║\n");
    printf("║                                                       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf(RESET "\n");

    // Criar gestores
    sys.aeroportos = gestor_aeroportos_criar();
    sys.avioes = gestor_avioes_criar();
    sys.voos = gestor_voos_criar();
    sys.passageiros = gestor_passageiros_criar();
    sys.reservas = gestor_reservas_criar();

    // Solicitar caminho do dataset
    printf("Introduza o caminho dos ficheiros de dados\n");
    printf("(deixe vazio para usar '%s'): ", DEFAULT_DATASET);

    if (fgets(caminho_dataset, sizeof(caminho_dataset), stdin))
    {
        limpar_input(caminho_dataset);

        if (strlen(caminho_dataset) == 0)
        {
            strcpy(caminho_dataset, DEFAULT_DATASET);
        }
    }
    else
    {
        strcpy(caminho_dataset, DEFAULT_DATASET);
    }

    // Carregar dataset
    carregar_dataset(&sys, caminho_dataset);

    // Loop principal
    while (1)
    {
        mostrar_menu();
        printf(BOLD "Escolha uma opção: " RESET);

        if (!fgets(opcao, sizeof(opcao), stdin))
            break;
        limpar_input(opcao);

        if (strlen(opcao) == 0)
            continue;

        int escolha = atoi(opcao);

        printf("\n");

        switch (escolha)
        {
        case 0:
            printf(YELLOW "Encerrando programa...\n" RESET);
            goto cleanup;

        case 1:
            executar_query1(&sys);
            break;

        case 2:
            executar_query2(&sys);
            break;

        case 3:
            executar_query3(&sys);
            break;

        case 4:
            executar_query4(&sys);
            break;

        case 5:
            executar_query5(&sys);
            break;

        case 6:
            executar_query6(&sys);
            break;

        default:
            printf(RED "✗ Opção inválida! Escolha entre 0-6.\n" RESET);
            break;
        }

        printf("\n" YELLOW "Pressione Enter para continuar..." RESET);
        getchar();
    }

cleanup:
    // Limpar memória
    gestor_aeroportos_destruir(sys.aeroportos);
    gestor_avioes_destruir(sys.avioes);
    gestor_voos_destruir(sys.voos);
    gestor_passageiros_destruir(sys.passageiros);
    gestor_reservas_destruir(sys.reservas);

    printf(GREEN "\n✓ Programa encerrado. Até breve!\n" RESET);
    return 0;
}