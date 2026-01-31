#include "gestor_teste.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

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

    if ((fgets(l1, sizeof(l1), f1) != NULL) || (fgets(l2, sizeof(l2), f2) != NULL))
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
static double memoria_pico_MB(void) __attribute__((unused));
static double memoria_pico_MB(void)
{
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
        return usage.ru_maxrss / 1024.0;
    return 0;
}

/**
 * @brief Gera um pequeno relatório JSON/HTML com métricas principais.
 */
static void escrever_relatorio_benchmark(const gestor_testes_t *gestor,
                                         const char *pasta_dataset,
                                         const char *ficheiro_input,
                                         const char *pasta_esperados,
                                         double tempo_execucao,
                                         double memoria_pico_mb,
                                         int total_ok,
                                         int total_testes)
{
    if (!gestor || !pasta_dataset || !ficheiro_input || !pasta_esperados)
        return;

    FILE *fjson = fopen("resultados/benchmark.json", "w");
    if (fjson)
    {
        fprintf(fjson,
                "{\n"
                "  \"dataset\": \"%s\",\n"
                "  \"input\": \"%s\",\n"
                "  \"esperados\": \"%s\",\n"
                "  \"tempo_execucao_s\": %.3f,\n"
                "  \"memoria_pico_mb\": %.1f,\n"
                "  \"testes_ok\": %d,\n"
                "  \"testes_total\": %d,\n"
                "  \"queries\": [\n",
                pasta_dataset, ficheiro_input, pasta_esperados,
                tempo_execucao, memoria_pico_mb, total_ok, total_testes);

        int first = 1;
        for (int i = 1; i < MAX_QUERIES; i++)
        {
            if (gestor->stats[i].total <= 0)
                continue;
            double tempo_medio = gestor->stats[i].tempo_total / gestor->stats[i].total;
            double perc = (gestor->stats[i].total > 0) ? (100.0 * gestor->stats[i].corretos / gestor->stats[i].total) : 0.0;

            fprintf(fjson,
                    "%s    {\"query\": %d, \"corretos\": %d, \"total\": %d, \"percentagem\": %.2f, \"tempo_medio_ms\": %.3f, \"tempo_min_ms\": %.3f, \"tempo_max_ms\": %.3f}\n",
                    first ? "" : ",\n",
                    i,
                    gestor->stats[i].corretos,
                    gestor->stats[i].total,
                    perc,
                    tempo_medio,
                    gestor->stats[i].tempo_min,
                    gestor->stats[i].tempo_max);
            first = 0;
        }

        fprintf(fjson, "  ]\n}\n");
        fclose(fjson);
    }

    FILE *fhtml = fopen("resultados/benchmark.html", "w");
    if (fhtml)
    {
        fprintf(fhtml,
                "<!doctype html>\n"
                "<html><head><meta charset=\"utf-8\"><title>Benchmark LI3</title>\n"
                "<style>body{font-family:Arial, sans-serif;padding:24px;}table{border-collapse:collapse;}td,th{border:1px solid #ccc;padding:8px;}</style>\n"
                "</head><body>\n"
                "<h2>Benchmark LI3</h2>\n"
                "<table>\n"
                "<tr><th>Dataset</th><td>%s</td></tr>\n"
                "<tr><th>Input</th><td>%s</td></tr>\n"
                "<tr><th>Esperados</th><td>%s</td></tr>\n"
                "<tr><th>Tempo execucao (s)</th><td>%.3f</td></tr>\n"
                "<tr><th>Memoria pico (MB)</th><td>%.1f</td></tr>\n"
                "<tr><th>Testes</th><td>%d/%d</td></tr>\n"
                "</table>\n",
                pasta_dataset, ficheiro_input, pasta_esperados,
                tempo_execucao, memoria_pico_mb, total_ok, total_testes);

        fprintf(fhtml,
                "<h3>Resultados por query</h3>\n"
                "<table>\n"
                "<tr><th>Query</th><th>Corretos</th><th>Total</th><th>Percentagem</th><th>Tempo médio (ms)</th><th>Tempo min (ms)</th><th>Tempo max (ms)</th></tr>\n");

        for (int i = 1; i < MAX_QUERIES; i++)
        {
            if (gestor->stats[i].total <= 0)
                continue;
            double tempo_medio = gestor->stats[i].tempo_total / gestor->stats[i].total;
            double perc = (gestor->stats[i].total > 0) ? (100.0 * gestor->stats[i].corretos / gestor->stats[i].total) : 0.0;
            fprintf(fhtml,
                    "<tr><td>Q%d</td><td>%d</td><td>%d</td><td>%.2f%%</td><td>%.3f</td><td>%.3f</td><td>%.3f</td></tr>\n",
                    i,
                    gestor->stats[i].corretos,
                    gestor->stats[i].total,
                    perc,
                    tempo_medio,
                    gestor->stats[i].tempo_min,
                    gestor->stats[i].tempo_max);
        }

        fprintf(fhtml, "</table>\n</body></html>\n");
        fclose(fhtml);
    }
}

/**
 * @brief Executa o programa-principal e devolve tempo e memória do processo filho.
 *
 * @param pasta_dataset Pasta do dataset.
 * @param ficheiro_input Ficheiro de input.
 * @param tempo_execucao_s Output do tempo em segundos.
 * @param memoria_pico_mb Output do pico de memória do processo filho (MB).
 * @return 0 em sucesso, -1 em erro.
 */
static int executar_programa_principal(const char *pasta_dataset,
                                       const char *ficheiro_input,
                                       double *tempo_execucao_s,
                                       double *memoria_pico_mb)
{
    if (!pasta_dataset || !ficheiro_input || !tempo_execucao_s || !memoria_pico_mb)
        return -1;

    double inicio = tempo_ms();
    pid_t pid = fork();
    if (pid < 0)
        return -1;

    if (pid == 0)
    {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0)
        {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        execl("./programa-principal", "programa-principal", pasta_dataset, ficheiro_input, (char *)NULL);
        _exit(127);
    }

    int status = 0;
    struct rusage usage;
    if (wait4(pid, &status, 0, &usage) < 0)
        return -1;

    double fim = tempo_ms();
    *tempo_execucao_s = (fim - inicio) / 1000.0;
    *memoria_pico_mb = usage.ru_maxrss / 1024.0;

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return -1;

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
    printf("\n");
    printf("==============================================================\n");
    printf("                RESUMO POR TIPO DE QUERY\n");
    printf("==============================================================\n\n");

    for (int i = 1; i < MAX_QUERIES; i++)
    {
        if (gestor->stats[i].total > 0)
        {
            double tempo_medio = gestor->stats[i].tempo_total / gestor->stats[i].total;

            printf("Q%d: %d/%d corretos", i, gestor->stats[i].corretos, gestor->stats[i].total);

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
    {
        for (int j = 0; j < gestor->num_queries_individuais - i - 1; j++)
        {
            if (gestor->queries_individuais[j].tempo_ms <
                gestor->queries_individuais[j + 1].tempo_ms)
            {
                InfoQueryIndividual temp = gestor->queries_individuais[j];
                gestor->queries_individuais[j] = gestor->queries_individuais[j + 1];
                gestor->queries_individuais[j + 1] = temp;
            }
        }
    }

    int limite = gestor->num_queries_individuais < 10 ? gestor->num_queries_individuais : 10;
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

    printf("A executar programa-principal...\n");
    double tempo_execucao = 0.0;
    double mem_pico = 0.0;
    if (executar_programa_principal(pasta_dataset, ficheiro_input, &tempo_execucao, &mem_pico) != 0)
    {
        fprintf(stderr, "Erro ao executar programa-principal.\n");
        return 1;
    }

    gestor->memoria_pico_MB = mem_pico;

    printf("Execucao completa em %.2f segundos\n", tempo_execucao);
    printf("Memoria pico (processo principal): %.1f MB\n\n", mem_pico);

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

    printf("Memoria atual (programa-testes): %.1f MB\n", mem_final);
    printf("Memoria pico (processo principal): %.1f MB\n", gestor->memoria_pico_MB);
    printf("Tempo execucao: %.2f s\n", tempo_execucao);
    printf("Tempo total: %.2f s\n",
           (tempo_fim_total - tempo_inicio_total) / 1000.0);

    escrever_relatorio_benchmark(gestor, pasta_dataset, ficheiro_input, pasta_esperados,
                                 tempo_execucao, gestor->memoria_pico_MB,
                                 gestor->total_ok, gestor->total_testes);

    printf("\n==============================================================\n\n");

    return (gestor->total_ok == gestor->total_testes) ? 0 : 1;
}
