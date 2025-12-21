#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>
#include <time.h>

#define MAX_LINHA 1024
#define MAX_QUERIES 10

typedef struct
{
    int total;
    int corretos;
    double tempo_total;
} EstatisticasQuery;

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

    printf(" A executar testes automáticos...\n");

    char comando[512];
    snprintf(comando, sizeof(comando),
             "./programa-principal %s %s > /dev/null 2>&1",
             dataset, ficheiro_input);

    printf(" A gerar resultados...\n");
    system(comando);

    EstatisticasQuery stats[MAX_QUERIES] = {0};
    int total_testes = 0;
    int total_ok = 0;

    printf("\n A comparar resultados...\n\n");

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
        total_testes++;

        if (cmp == 1)
        {
            stats[tipo].corretos++;
            total_ok++;
        }
        else if (cmp == 0)
        {
            printf(" Discrepância na query %d (tipo Q%d): linha %d de \"%s\"\n",
                   i, tipo, linha_dif, path_res);
        }
        else
        {
            printf(" Erro ao comparar query %d: ficheiro não encontrado\n", i);
        }
    }

    double tempo_fim_total = tempo_ms();
    double mem = memoria_MB();

    printf("\n");
    printf("═══════════════════════════════════════════════════\n");
    printf("            RESULTADOS DOS TESTES\n");
    printf("═══════════════════════════════════════════════════\n\n");

    for (int i = 1; i < MAX_QUERIES; i++)
    {
        if (stats[i].total > 0)
        {
            double tempo_medio = stats[i].tempo_total / stats[i].total;
            printf("Q%d: %d de %d testes OK",
                   i, stats[i].corretos, stats[i].total);

            if (stats[i].corretos == stats[i].total)
                printf(" ");
            else
                printf(" ");

            printf(" (%.2f ms médio)\n", tempo_medio);
        }
    }

    printf("\n");
    printf("───────────────────────────────────────────────────\n");
    printf("Total: %d de %d testes OK", total_ok, total_testes);

    if (total_ok == total_testes)
        printf(" \n");
    else
        printf(" (%d falhas)\n", total_testes - total_ok);

    printf("───────────────────────────────────────────────────\n");
    printf("Memória utilizada: %.1f MB\n", mem);
    printf("Tempo total: %.2f s\n", (tempo_fim_total - tempo_inicio_total) / 1000.0);
    printf("═══════════════════════════════════════════════════\n");

    return (total_ok == total_testes) ? 0 : 1;
}