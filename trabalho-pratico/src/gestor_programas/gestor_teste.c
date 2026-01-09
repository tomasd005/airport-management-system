#include "gestor_teste.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_LINHA 1024
#define MAX_QUERIES 10
#define MAX_INDIVIDUAL_QUERIES 1000

/**
 * @struct EstatisticasQuery
 * @brief Estatísticas agregadas por tipo de query.
 */
typedef struct
{
    int total;
    int corretos;
    double tempo_total;
    double tempo_min;
    double tempo_max;
} EstatisticasQuery;

/**
 * @struct InfoQueryIndividual
 * @brief Informação detalhada sobre uma query individual.
 */
typedef struct
{
    int comando_num;
    int tipo_query;
    double tempo_ms;
    int correto;
} InfoQueryIndividual;

/**
 * @struct GestorTestes
 * @brief Estrutura principal do gestor de testes.
 *
 * Armazena estatísticas globais, resultados individuais e
 * informação sobre utilização de memória.
 */
struct GestorTestes
{
    EstatisticasQuery stats[MAX_QUERIES];
    InfoQueryIndividual queries_individuais[MAX_INDIVIDUAL_QUERIES];
    int num_queries_individuais;
    int total_testes;
    int total_ok;
    double memoria_pico_MB;
};

/**
 * @brief Cria e inicializa o gestor de testes.
 *
 * @return Ponteiro para o gestor criado ou NULL em caso de erro
 */
gestor_testes_t *gestor_testes_criar(void)
{
    gestor_testes_t *gestor = malloc(sizeof(gestor_testes_t));
    if (!gestor)
        return NULL;

    memset(gestor, 0, sizeof(gestor_testes_t));

    for (int i = 0; i < MAX_QUERIES; i++)
    {
        gestor->stats[i].tempo_min = 1e9;
        gestor->stats[i].tempo_max = 0;
    }

    return gestor;
}

/**
 * @brief Liberta a memória associada ao gestor de testes.
 *
 * @param gestor Gestor de testes
 */
void gestor_testes_destruir(gestor_testes_t *gestor)
{
    free(gestor);
}

/**
 * @brief Compara dois ficheiros linha a linha.
 *
 * @param ficheiro1 Caminho do primeiro ficheiro
 * @param ficheiro2 Caminho do segundo ficheiro
 * @param linha_diferente Linha onde ocorre a primeira diferença
 *
 * @return 1 se forem iguais, 0 se diferentes, -1 em erro
 */
static int comparar_ficheiros(
    const char *ficheiro1,
    const char *ficheiro2,
    int *linha_diferente)
{
    FILE *f1 = fopen(ficheiro1, "r");
    FILE *f2 = fopen(ficheiro2, "r");

    if (!f1 || !f2)
    {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return -1;
    }

    char l1[MAX_LINHA], l2[MAX_LINHA];
    int linha = 1;

    while (fgets(l1, sizeof(l1), f1) &&
           fgets(l2, sizeof(l2), f2))
    {
        if (strcmp(l1, l2) != 0)
        {
            *linha_diferente = linha;
            fclose(f1);
            fclose(f2);
            return 0;
        }
        linha++;
    }

    if (fgets(l1, sizeof(l1), f1) ||
        fgets(l2, sizeof(l2), f2))
    {
        *linha_diferente = linha;
        fclose(f1);
        fclose(f2);
        return 0;
    }

    fclose(f1);
    fclose(f2);
    return 1;
}

/**
 * @brief Obtém o tempo atual em milissegundos.
 *
 * @return Tempo atual (ms)
 */
static double tempo_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

/**
 * @brief Obtém a memória residente atual do processo.
 *
 * @return Memória atual em MB
 */
static double memoria_atual_MB(void)
{
    FILE *f = fopen("/proc/self/status", "r");
    if (!f)
        return 0;

    char linha[256];
    double mem_kB = 0;

    while (fgets(linha, sizeof(linha), f))
    {
        if (strncmp(linha, "VmRSS:", 6) == 0)
        {
            sscanf(linha + 6, "%lf", &mem_kB);
            break;
        }
    }

    fclose(f);
    return mem_kB / 1024.0;
}

/**
 * @brief Obtém o pico de memória utilizado pelo processo.
 *
 * @return Pico de memória em MB
 */
static double memoria_pico_MB(void)
{
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
        return usage.ru_maxrss / 1024.0;
    return 0;
}

/**
 * @brief Determina o tipo de query de um comando específico.
 *
 * @param ficheiro_input Ficheiro de input das queries
 * @param comando_num Número do comando
 *
 * @return Tipo da query ou -1 em erro
 */
static int detectar_tipo_query(
    const char *ficheiro_input,
    int comando_num)
{
    FILE *f = fopen(ficheiro_input, "r");
    if (!f)
        return -1;

    char linha[256];
    int contador = 1;

    while (fgets(linha, sizeof(linha), f))
    {
        if (linha[0] == '\n' || linha[0] == '\r' || linha[0] == '\0')
            continue;

        if (contador == comando_num)
        {
            char *tipo = strtok(linha, " ,\t;\r\n");
            if (tipo)
            {
                fclose(f);
                return atoi(tipo);
            }
        }
        contador++;
    }

    fclose(f);
    return -1;
}

/**
 * @brief Imprime um resumo estatístico por tipo de query.
 *
 * @param gestor Gestor de testes
 */
static void imprimir_resumo_queries(gestor_testes_t *gestor)
{
    printf("\n==============================================================\n");
    printf("                RESUMO POR TIPO DE QUERY\n");
    printf("==============================================================\n\n");

    for (int i = 1; i < MAX_QUERIES; i++)
    {
        if (gestor->stats[i].total > 0)
        {
            double media = gestor->stats[i].tempo_total /
                            gestor->stats[i].total;

            printf("Q%d: %d/%d corretos %s\n",
                   i,
                   gestor->stats[i].corretos,
                   gestor->stats[i].total,
                   (gestor->stats[i].corretos == gestor->stats[i].total) ? "[OK]" : "[FAIL]");

            if (gestor->stats[i].corretos == gestor->stats[i].total)
                printf(" [OK]");
            else
                printf(" [FAIL]");

            printf("\n");
            printf("    Tempo medio: %8.2f ms\n", tempo_medio);
            printf("    Tempo min:   %8.2f ms\n", gestor->stats[i].tempo_min);
            printf("    Tempo max:   %8.2f ms\n", gestor->stats[i].tempo_max);
            printf("    Total:       %8.2f ms\n\n", gestor->stats[i].tempo_total);
        }
    }
}

/**
 * @brief Imprime as queries individuais mais lentas.
 *
 * @param gestor Gestor de testes
 */
static void imprimir_queries_lentas(gestor_testes_t *gestor)
{
    printf("==============================================================\n");
    printf("                  QUERIES MAIS LENTAS\n");
    printf("==============================================================\n\n");

    for (int i = 0; i < gestor->num_queries_individuais - 1; i++)
        for (int j = 0; j < gestor->num_queries_individuais - i - 1; j++)
            if (gestor->queries_individuais[j].tempo_ms <
                gestor->queries_individuais[j + 1].tempo_ms)
            {
                InfoQueryIndividual tmp = gestor->queries_individuais[j];
                gestor->queries_individuais[j] = gestor->queries_individuais[j + 1];
                gestor->queries_individuais[j + 1] = tmp;
            }

    int limite = gestor->num_queries_individuais < 10
                 ? gestor->num_queries_individuais
                 : 10;

    for (int i = 0; i < limite; i++)
    {
        InfoQueryIndividual *q = &gestor->queries_individuais[i];
        const char *status = q->correto ? "[OK]" : "[FAIL]";
        printf("%2d. Command %3d (Q%d): %8.2f ms %s\n",
               i + 1, q->comando_num, q->tipo_query, q->tempo_ms, status);
    }
    printf("\n");
}

/**
 * @brief Executa os testes automáticos completos.
 *
 * @param gestor Gestor de testes
 * @param pasta_dataset Pasta com o dataset
 * @param ficheiro_input Ficheiro de queries
 * @param pasta_esperados Pasta com outputs esperados
 *
 * @return 0 se todos os testes passarem, 1 caso contrário
 */
int gestor_testes_executar(
    gestor_testes_t *gestor,
    const char *pasta_dataset,
    const char *ficheiro_input,
    const char *pasta_esperados)
{
    double tempo_inicio_total = tempo_ms();

    printf("\n==============================================================\n");
    printf("         TESTES AUTOMATICOS - LI3 2025/2026                  \n");
    printf("==============================================================\n\n");

    printf("Dataset: %s\n", pasta_dataset);
    printf("Input: %s\n", ficheiro_input);
    printf("Esperados: %s\n\n", pasta_esperados);

    /* Construção do comando para executar o programa principal */
    char comando[512];
    snprintf(comando, sizeof(comando),
             "./programa-principal %s %s > /dev/null 2>&1",
             pasta_dataset, ficheiro_input);

    printf("A executar programa-principal...\n");

    double tempo_exec_inicio = tempo_ms();
    double mem_antes = memoria_atual_MB();

    /* Execução do programa principal */
    system(comando);

    double tempo_exec_fim = tempo_ms();
    double tempo_execucao = (tempo_exec_fim - tempo_exec_inicio) / 1000.0;
    double mem_pico = memoria_pico_MB();

    gestor->memoria_pico_MB = mem_pico;

    printf("Execucao completa em %.2f segundos\n", tempo_execucao);
    printf("Memoria pico: %.1f MB\n\n", mem_pico);

    printf("==============================================================\n");
    printf("                  COMPARANDO RESULTADOS\n");
    printf("==============================================================\n\n");

    /* Contar número de comandos no ficheiro de input */
    int max_comandos = 0;
    FILE *f_count = fopen(ficheiro_input, "r");
    if (!f_count)
    {
        fprintf(stderr, "Erro ao abrir ficheiro de input: %s\n", ficheiro_input);
        return 1;
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), f_count))
    {
        if (linha[0] != '\n' && linha[0] != '\r' && linha[0] != '\0')
            max_comandos++;
    }
    fclose(f_count);

    /* Comparar cada resultado com o esperado */
    for (int i = 1; i <= max_comandos; i++)
    {
        char path_res[256], path_exp[256];

        snprintf(path_res, sizeof(path_res),
                 "resultados/command%d_output.txt", i);
        snprintf(path_exp, sizeof(path_exp),
                 "%s/command%d_output.txt", pasta_esperados, i);

        int tipo = detectar_tipo_query(ficheiro_input, i);

        if (tipo < 0 || tipo >= MAX_QUERIES)
            continue;

        double tempo_inicio = tempo_ms();

        int linha_dif = 0;
        int cmp = comparar_ficheiros(path_res, path_exp, &linha_dif);

        double tempo_fim = tempo_ms();
        double tempo_query = tempo_fim - tempo_inicio;

        /* Atualizar estatísticas */
        gestor->stats[tipo].total++;
        gestor->stats[tipo].tempo_total += tempo_query;

        if (tempo_query < gestor->stats[tipo].tempo_min)
            gestor->stats[tipo].tempo_min = tempo_query;

        if (tempo_query > gestor->stats[tipo].tempo_max)
            gestor->stats[tipo].tempo_max = tempo_query;

        gestor->total_testes++;

        int correto = (cmp == 1);

        /* Guardar informação individual */
        if (gestor->num_queries_individuais < MAX_INDIVIDUAL_QUERIES)
        {
            InfoQueryIndividual *q =
                &gestor->queries_individuais[gestor->num_queries_individuais++];

            q->comando_num = i;
            q->tipo_query = tipo;
            q->tempo_ms = tempo_query;
            q->correto = correto;
        }

        if (correto)
        {
            gestor->stats[tipo].corretos++;
            gestor->total_ok++;
        }
        else if (cmp == 0)
        {
            printf("[X] Query %d (Q%d): ERRO na linha %d\n",
                   i, tipo, linha_dif);
        }
        else
        {
            printf("[!] Query %d (Q%d): Ficheiro nao encontrado\n",
                   i, tipo);
        }
    }

    double tempo_fim_total = tempo_ms();
    double mem_final = memoria_atual_MB();

    /* Impressão dos relatórios */
    imprimir_resumo_queries(gestor);
    imprimir_queries_lentas(gestor);

    printf("==============================================================\n");
    printf("                     RESUMO FINAL\n");
    printf("==============================================================\n\n");

    printf("Testes: %d/%d corretos",
           gestor->total_ok, gestor->total_testes);

    if (gestor->total_ok == gestor->total_testes)
        printf(" [OK]\n");
    else
        printf(" (%d falhas)\n",
               gestor->total_testes - gestor->total_ok);

    printf("Memoria atual: %.1f MB\n", mem_final);
    printf("Memoria pico: %.1f MB\n", gestor->memoria_pico_MB);
    printf("Tempo execucao: %.2f s\n", tempo_execucao);
    printf("Tempo total: %.2f s\n",
           (tempo_fim_total - tempo_inicio_total) / 1000.0);

    printf("\n==============================================================\n\n");

    return (gestor->total_ok == gestor->total_testes) ? 0 : 1;
}

