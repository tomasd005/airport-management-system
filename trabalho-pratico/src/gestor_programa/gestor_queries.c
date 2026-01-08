#include "gestor_queries.h"
#include "../queries/querie1.h"
#include "../queries/querie2.h"
#include "../queries/querie3.h"
#include "../queries/querie4.h"
#include "../queries/querie5.h"
#include "../queries/querie6.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct gestor_queries
{
    gestor_aeroportos_t *aeroportos;
    gestor_avioes_t *avioes;
    gestor_voos_t *voos;
    gestor_passageiros_t *passageiros;
    gestor_reservas_t *reservas;
};

gestor_queries_t *gestor_queries_criar(
    gestor_aeroportos_t *aeroportos,
    gestor_avioes_t *avioes,
    gestor_voos_t *voos,
    gestor_passageiros_t *passageiros,
    gestor_reservas_t *reservas)
{
    gestor_queries_t *g = g_new0(gestor_queries_t, 1);
    g->aeroportos = aeroportos;
    g->avioes = avioes;
    g->voos = voos;
    g->passageiros = passageiros;
    g->reservas = reservas;
    return g;
}

void gestor_queries_destruir(gestor_queries_t *gestor)
{
    if (!gestor)
        return;
    g_free(gestor);
}

static int obter_numero_query(const char *linha)
{
    if (!linha)
        return -1;

    while (*linha && isspace(*linha))
        linha++;

    if (!isdigit(*linha))
        return -1;

    return atoi(linha);
}

static void processar_query1(gestor_queries_t *g, const char *linha, FILE *output)
{
    char *copia = g_strdup(linha);
    char *token = strtok(copia, " \t");
    if (!token)
    {
        fprintf(output, "\n");
        g_free(copia);
        return;
    }

    token = strtok(NULL, " \t");
    if (!token)
    {
        fprintf(output, "\n");
        g_free(copia);
        return;
    }

    if (strcmp(token, "S") == 0 || strcmp(token, "s") == 0)
    {
        token = strtok(NULL, " \t\r\n");
        if (!token)
        {
            fprintf(output, "\n");
            g_free(copia);
            return;
        }
    }

    query1(g->aeroportos, g->voos, g->reservas, linha, token, output);
    g_free(copia);
}

static void processar_query2(gestor_queries_t *g, const char *linha, FILE *output)
{
    char *copia = g_strdup(linha);
    char *saveptr = NULL;
    char *token = strtok_r(copia, " \t", &saveptr);
    if (!token)
    {
        fprintf(output, "\n");
        g_free(copia);
        return;
    }

    token = strtok_r(NULL, " \t", &saveptr);
    if (!token)
    {
        fprintf(output, "\n");
        g_free(copia);
        return;
    }

    if (strcmp(token, "S") == 0 || strcmp(token, "s") == 0)
    {
        token = strtok_r(NULL, " \t", &saveptr);
        if (!token)
        {
            fprintf(output, "\n");
            g_free(copia);
            return;
        }
    }

    int N = atoi(token);

    const char *fabricante = saveptr ? saveptr : "";
    char *fab_copy = g_strdup(fabricante);
    fab_copy[strcspn(fab_copy, "\r\n")] = '\0';

    query2(g->avioes, g->voos, N, fab_copy, linha, output);
    g_free(fab_copy);
    g_free(copia);
}

static void processar_query3(gestor_queries_t *g, const char *linha, FILE *output)
{
    char *copia = g_strdup(linha);
    char *saveptr = NULL;
    char *token = strtok_r(copia, " \t", &saveptr);
    if (!token)
    {
        fprintf(output, "\n");
        g_free(copia);
        return;
    }

    token = strtok_r(NULL, " \t", &saveptr);
    if (!token)
    {
        fprintf(output, "\n");
        g_free(copia);
        return;
    }

    if (strcmp(token, "S") == 0 || strcmp(token, "s") == 0)
    {
        token = strtok_r(NULL, " \t", &saveptr);
        if (!token)
        {
            fprintf(output, "\n");
            g_free(copia);
            return;
        }
    }

    char *data_inicio = g_strdup(token);

    token = strtok_r(NULL, " \t\r\n", &saveptr);
    if (!token)
    {
        fprintf(output, "\n");
        g_free(data_inicio);
        g_free(copia);
        return;
    }
    char *data_fim = g_strdup(token);

    query3(g->aeroportos, g->voos, data_inicio, data_fim, linha, output);
    g_free(data_inicio);
    g_free(data_fim);
    g_free(copia);
}

static void processar_query4(gestor_queries_t *g, const char *linha, FILE *output)
{
    char *copia = g_strdup(linha);
    char *saveptr = NULL;
    char *token = strtok_r(copia, " \t", &saveptr);
    if (!token)
    {
        fprintf(output, "\n");
        g_free(copia);
        return;
    }

    token = strtok_r(NULL, " \t", &saveptr);
    if (!token)
    {
        query4(g->reservas, g->voos, g->passageiros, NULL, NULL, linha, output);
        g_free(copia);
        return;
    }

    if (strcmp(token, "S") == 0 || strcmp(token, "s") == 0)
    {
        token = strtok_r(NULL, " \t", &saveptr);
        if (!token)
        {
            query4(g->reservas, g->voos, g->passageiros, NULL, NULL, linha, output);
            g_free(copia);
            return;
        }
    }

    char *data_inicio = g_strdup(token);

    token = strtok_r(NULL, " \t\r\n", &saveptr);
    if (!token)
    {
        fprintf(output, "\n");
        g_free(data_inicio);
        g_free(copia);
        return;
    }
    char *data_fim = g_strdup(token);

    query4(g->reservas, g->voos, g->passageiros, data_inicio, data_fim, linha, output);
    g_free(data_inicio);
    g_free(data_fim);
    g_free(copia);
}

static void processar_query5(gestor_queries_t *g, const char *linha, FILE *output)
{
    char *copia = g_strdup(linha);
    char *saveptr = NULL;
    char *token = strtok_r(copia, " \t", &saveptr);
    if (!token)
    {
        fprintf(output, "\n");
        g_free(copia);
        return;
    }

    token = strtok_r(NULL, " \t", &saveptr);
    if (!token)
    {
        fprintf(output, "\n");
        g_free(copia);
        return;
    }

    if (strcmp(token, "S") == 0 || strcmp(token, "s") == 0)
    {
        token = strtok_r(NULL, " \t\r\n", &saveptr);
        if (!token)
        {
            fprintf(output, "\n");
            g_free(copia);
            return;
        }
    }

    int N = atoi(token);

    query5(g->voos, N, linha, output);
    g_free(copia);
}

static void processar_query6(gestor_queries_t *g, const char *linha, FILE *output)
{
    const char *p = linha;

    while (*p && isspace(*p))
        p++;
    while (*p && isdigit(*p))
        p++;
    while (*p && isspace(*p))
        p++;

    if ((*p == 'S' || *p == 's') && (*(p + 1) == ' ' || *(p + 1) == '\t'))
    {
        p++;
        while (*p && isspace(*p))
            p++;
    }

    if (!*p)
    {
        fprintf(output, "\n");
        return;
    }

    char *nacionalidade = g_strdup(p);
    nacionalidade[strcspn(nacionalidade, "\r\n")] = '\0';

    query6(g->reservas, g->voos, g->passageiros, nacionalidade, linha, output);
    g_free(nacionalidade);
}

void gestor_queries_processar_ficheiro(gestor_queries_t *gestor, const char *ficheiro_input)
{
    if (!gestor || !ficheiro_input)
        return;

    FILE *input = fopen(ficheiro_input, "r");
    if (!input)
        return;

    char linha[1024];
    int numero_comando = 1;

    while (fgets(linha, sizeof(linha), input))
    {
        linha[strcspn(linha, "\r\n")] = '\0';

        if (strlen(linha) == 0)
            continue;

        int numero_query = obter_numero_query(linha);

        char nome_output[256];
        snprintf(nome_output, sizeof(nome_output), "resultados/command%d_output.txt", numero_comando);

        FILE *output = fopen(nome_output, "w");
        if (!output)
        {
            numero_comando++;
            continue;
        }

        switch (numero_query)
        {
        case 1:
            processar_query1(gestor, linha, output);
            break;
        case 2:
            processar_query2(gestor, linha, output);
            break;
        case 3:
            processar_query3(gestor, linha, output);
            break;
        case 4:
            processar_query4(gestor, linha, output);
            break;
        case 5:
            processar_query5(gestor, linha, output);
            break;
        case 6:
            processar_query6(gestor, linha, output);
            break;
        default:
            fprintf(output, "\n");
            break;
        }

        fclose(output);
        numero_comando++;
    }

    fclose(input);
}