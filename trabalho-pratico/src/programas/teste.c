#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>

#define MAX_LINHA 1024
#define MAX_QUERIES 10
#define MAX_INDIVIDUAL_QUERIES 1000

typedef struct
{
    int total;
    int corretos;
    double tempo_total;
    double tempo_min;
    double tempo_max;
} EstatisticasQuery;

typedef struct
{
    int comando_num;
    int tipo_query;
    double tempo_ms;
    int correto;
} InfoQueryIndividual;

int comparar_ficheiros(const char *ficheiro1, const char *ficheiro2, int *linha_diferente)
{
    FILE *f1 = fopen(ficheiro1, "r");
    FILE *f2 = fopen(ficheiro2, "r");

    if (!f1 || !f2)
    {
        if (f1)
            fclose(f1);
        if (f2)
            fclose(f2);
        return -1;
    }

    char l1[MAX_LINHA], l2[MAX_LINHA];
    int linha = 1;

    while (fgets(l1, sizeof(l1), f1) && fgets(l2, sizeof(l2), f2))
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

double tempo_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

double memoria_MB()
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

int detectar_tipo_query(const char *ficheiro_input, int comando_num)
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

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Uso: %s <pasta_datasets> <ficheiro_input> <pasta_esperados>\n", argv[0]);
        return 1;
    }

    const char *dataset = argv[1];
    const char *ficheiro_input = argv[2];
    const char *esperados = argv[3];

    double tempo_inicio_total = tempo_ms();

    printf("\n==============================================================\n");
    printf("         TESTES AUTOMATICOS - LI3 2024/2025                  \n");
    printf("==============================================================\n\n");

    printf("Dataset: %s\n", dataset);
    printf("Input: %s\n", ficheiro_input);
    printf("Esperados: %s\n\n", esperados);

    char comando[512];
    snprintf(comando, sizeof(comando),
             "./programa-principal %s %s > /dev/null 2>&1",
             dataset, ficheiro_input);

    printf("A executar programa-principal...\n");
    double tempo_exec_inicio = tempo_ms();
    system(comando);
    double tempo_exec_fim = tempo_ms();
    double tempo_execucao = (tempo_exec_fim - tempo_exec_inicio) / 1000.0;

    printf("Execucao completa em %.2f segundos\n\n", tempo_execucao);

    EstatisticasQuery stats[MAX_QUERIES] = {0};
    InfoQueryIndividual queries_individuais[MAX_INDIVIDUAL_QUERIES];
    int num_queries_individuais = 0;

    int total_testes = 0;
    int total_ok = 0;

    // Inicializar min/max
    for (int i = 0; i < MAX_QUERIES; i++)
    {
        stats[i].tempo_min = 1e9;
        stats[i].tempo_max = 0;
    }

    printf("==============================================================\n");
    printf("                  COMPARANDO RESULTADOS\n");
    printf("==============================================================\n\n");

    int max_comandos = 0;
    FILE *f_count = fopen(ficheiro_input, "r");
    if (f_count)
    {
        char linha[256];
        while (fgets(linha, sizeof(linha), f_count))
        {
            if (linha[0] != '\n' && linha[0] != '\r' && linha[0] != '\0')
                max_comandos++;
        }
        fclose(f_count);
    }

    for (int i = 1; i <= max_comandos; i++)
    {
        char path_res[256], path_exp[256];
        snprintf(path_res, sizeof(path_res), "resultados/command%d_output.txt", i);
        snprintf(path_exp, sizeof(path_exp), "%s/command%d_output.txt", esperados, i);

        int tipo = detectar_tipo_query(ficheiro_input, i);

        if (tipo < 0 || tipo >= MAX_QUERIES)
            continue;

        double tempo_inicio = tempo_ms();

        int linha_dif = 0;
        int cmp = comparar_ficheiros(path_res, path_exp, &linha_dif);

        double tempo_fim = tempo_ms();
        double tempo_query = tempo_fim - tempo_inicio;

        stats[tipo].total++;
        stats[tipo].tempo_total += tempo_query;

        if (tempo_query < stats[tipo].tempo_min)
            stats[tipo].tempo_min = tempo_query;
        if (tempo_query > stats[tipo].tempo_max)
            stats[tipo].tempo_max = tempo_query;

        total_testes++;

        int correto = (cmp == 1);

        if (num_queries_individuais < MAX_INDIVIDUAL_QUERIES)
        {
            queries_individuais[num_queries_individuais].comando_num = i;
            queries_individuais[num_queries_individuais].tipo_query = tipo;
            queries_individuais[num_queries_individuais].tempo_ms = tempo_query;
            queries_individuais[num_queries_individuais].correto = correto;
            num_queries_individuais++;
        }

        if (correto)
        {
            stats[tipo].corretos++;
            total_ok++;
        }
        else if (cmp == 0)
        {
            printf("[X] Query %d (Q%d): ERRO na linha %d\n", i, tipo, linha_dif);
        }
        else
        {
            printf("[!] Query %d (Q%d): Ficheiro nao encontrado\n", i, tipo);
        }
    }

    double tempo_fim_total = tempo_ms();
    double mem = memoria_MB();

    printf("\n");
    printf("==============================================================\n");
    printf("                RESUMO POR TIPO DE QUERY\n");
    printf("==============================================================\n\n");

    for (int i = 1; i < MAX_QUERIES; i++)
    {
        if (stats[i].total > 0)
        {
            double tempo_medio = stats[i].tempo_total / stats[i].total;

            printf("Q%d: %d/%d corretos", i, stats[i].corretos, stats[i].total);

            if (stats[i].corretos == stats[i].total)
                printf(" [OK]");
            else
                printf(" [FAIL]");

            printf("\n");
            printf("    Tempo medio: %8.2f ms\n", tempo_medio);
            printf("    Tempo min:   %8.2f ms\n", stats[i].tempo_min);
            printf("    Tempo max:   %8.2f ms\n", stats[i].tempo_max);
            printf("    Total:       %8.2f ms\n\n", stats[i].tempo_total);
        }
    }

    printf("==============================================================\n");
    printf("                  QUERIES MAIS LENTAS\n");
    printf("==============================================================\n\n");

    // Ordenar queries por tempo (bubble sort)
    for (int i = 0; i < num_queries_individuais - 1; i++)
    {
        for (int j = 0; j < num_queries_individuais - i - 1; j++)
        {
            if (queries_individuais[j].tempo_ms < queries_individuais[j + 1].tempo_ms)
            {
                InfoQueryIndividual temp = queries_individuais[j];
                queries_individuais[j] = queries_individuais[j + 1];
                queries_individuais[j + 1] = temp;
            }
        }
    }

    // Mostrar top 10 mais lentas
    int limite = num_queries_individuais < 10 ? num_queries_individuais : 10;
    for (int i = 0; i < limite; i++)
    {
        InfoQueryIndividual *q = &queries_individuais[i];
        const char *status = q->correto ? "[OK]" : "[FAIL]";
        printf("%2d. Command %3d (Q%d): %8.2f ms %s\n",
               i + 1, q->comando_num, q->tipo_query, q->tempo_ms, status);
    }

    printf("\n");
    printf("==============================================================\n");
    printf("                     RESUMO FINAL\n");
    printf("==============================================================\n\n");

    printf("Testes: %d/%d corretos", total_ok, total_testes);
    if (total_ok == total_testes)
        printf(" [OK]\n");
    else
        printf(" (%d falhas)\n", total_testes - total_ok);

    printf("Memoria: %.1f MB\n", mem);
    printf("Tempo execucao: %.2f s\n", tempo_execucao);
    printf("Tempo total: %.2f s\n", (tempo_fim_total - tempo_inicio_total) / 1000.0);

    printf("\n==============================================================\n\n");

    return (total_ok == total_testes) ? 0 : 1;
}