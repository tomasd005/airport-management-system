#include "gestor_principal.h"
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

struct GestorPrincipal
{
    gestor_aeroportos_t *aeroportos;
    gestor_avioes_t *avioes;
    gestor_voos_t *voos;
    gestor_passageiros_t *passageiros;
    gestor_reservas_t *reservas;
};

gestor_principal_t *gestor_principal_criar(void)
{
    gestor_principal_t *gestor = malloc(sizeof(gestor_principal_t));
    if (!gestor)
        return NULL;

    gestor->aeroportos = gestor_aeroportos_criar();
    gestor->avioes = gestor_avioes_criar();
    gestor->voos = gestor_voos_criar();
    gestor->passageiros = gestor_passageiros_criar();
    gestor->reservas = gestor_reservas_criar();

    return gestor;
}

void gestor_principal_destruir(gestor_principal_t *gestor)
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

static void trim_string(char *str)
{
    if (!str)
        return;

    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }

    char *start = str;
    while (*start && isspace((unsigned char)*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);
}

static void carregar_dados(gestor_principal_t *gestor, const char *pasta)
{
    char caminho[512];

    snprintf(caminho, sizeof(caminho), "%s/airports.csv", pasta);
    gestor_aeroportos_carregar(gestor->aeroportos, caminho);

    snprintf(caminho, sizeof(caminho), "%s/aircrafts.csv", pasta);
    gestor_avioes_carregar(gestor->avioes, caminho);

    snprintf(caminho, sizeof(caminho), "%s/flights.csv", pasta);
    gestor_voos_carregar(gestor->voos, caminho);

    snprintf(caminho, sizeof(caminho), "%s/passengers.csv", pasta);
    gestor_passageiros_carregar(gestor->passageiros, caminho);

    snprintf(caminho, sizeof(caminho), "%s/reservations.csv", pasta);
    gestor_reservas_carregar_com_validacao(
        gestor->reservas, caminho,
        gestor->voos, gestor->passageiros);
}

static void executar_query(
    gestor_principal_t *gestor,
    const char *linha_completa,
    FILE *output)
{
    char linha[256];
    strncpy(linha, linha_completa, sizeof(linha) - 1);
    linha[sizeof(linha) - 1] = '\0';

    char *p = linha;

    // Skip whitespace
    while (*p && isspace((unsigned char)*p))
        p++;

    if (!*p)
    {
        fprintf(output, "\n");
        return;
    }

    // Extract query type
    char *tipo_str = p;
    while (*p && !isspace((unsigned char)*p))
        p++;

    if (*p)
        *p++ = '\0';

    int tipo = atoi(tipo_str);

    // Execute query based on type
    switch (tipo)
    {
    case 1:
    {
        while (*p && isspace((unsigned char)*p))
            p++;
        char *aeroporto = p;
        trim_string(aeroporto);

        query1(gestor->aeroportos, gestor->voos, gestor->reservas,
               linha_completa, aeroporto, output);
        break;
    }

    case 2:
    {
        while (*p && isspace((unsigned char)*p))
            p++;

        char *n_str = p;
        while (*p && !isspace((unsigned char)*p))
            p++;
        if (*p)
            *p++ = '\0';

        int N = atoi(n_str);

        while (*p && isspace((unsigned char)*p))
            p++;

        char *fabricante = p;
        trim_string(fabricante);

        if (strlen(fabricante) == 0)
            fabricante = NULL;

        query2(gestor->avioes, gestor->voos, N, fabricante, linha_completa, output);
        break;
    }

    case 3:
    {
        while (*p && isspace((unsigned char)*p))
            p++;

        char *data_inicio = p;
        while (*p && !isspace((unsigned char)*p))
            p++;
        if (*p)
            *p++ = '\0';

        while (*p && isspace((unsigned char)*p))
            p++;

        char *data_fim = p;
        trim_string(data_fim);

        if (strlen(data_inicio) > 0 && strlen(data_fim) > 0)
        {
            query3(gestor->aeroportos, gestor->voos,
                   data_inicio, data_fim, linha_completa, output);
        }
        else
        {
            fprintf(output, "\n");
        }
        break;
    }

    case 4:
    {
        while (*p && isspace((unsigned char)*p))
            p++;

        char *data_inicio = NULL;
        char *data_fim = NULL;

        if (*p && !isspace((unsigned char)*p))
        {
            data_inicio = p;
            while (*p && !isspace((unsigned char)*p))
                p++;
            if (*p)
                *p++ = '\0';

            while (*p && isspace((unsigned char)*p))
                p++;

            if (*p)
            {
                data_fim = p;
                trim_string(data_fim);
            }
        }

        query4(gestor->reservas, gestor->voos, gestor->passageiros,
               data_inicio, data_fim, linha_completa, output);
        break;
    }

    case 5:
    {
        while (*p && isspace((unsigned char)*p))
            p++;

        char *n_str = p;
        trim_string(n_str);
        int N = atoi(n_str);

        query5(gestor->voos, N, linha_completa, output);
        break;
    }

    case 6:
    {
        while (*p && isspace((unsigned char)*p))
            p++;

        char *nacionalidade = p;
        trim_string(nacionalidade);

        if (strlen(nacionalidade) > 0)
        {
            query6(gestor->reservas, gestor->voos, gestor->passageiros,
                   nacionalidade, linha_completa, output);
        }
        else
        {
            fprintf(output, "\n");
        }
        break;
    }

    default:
        fprintf(output, "\n");
        break;
    }
}

void gestor_principal_executar(
    gestor_principal_t *gestor,
    const char *pasta_dados,
    const char *ficheiro_input)
{
    if (!gestor || !pasta_dados || !ficheiro_input)
        return;

    // Carregar dados
    carregar_dados(gestor, pasta_dados);

    // Abrir ficheiro de input
    FILE *input = fopen(ficheiro_input, "r");
    if (!input)
    {
        perror("Erro ao abrir ficheiro de input");
        return;
    }

    // Processar queries
    char linha[256];
    int contador = 1;

    while (fgets(linha, sizeof(linha), input))
    {
        // Remove newlines
        size_t len = strlen(linha);
        while (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r'))
        {
            linha[len - 1] = '\0';
            len--;
        }

        if (len == 0)
            continue;

        // Create output file
        char caminho_saida[256];
        snprintf(caminho_saida, sizeof(caminho_saida),
                 "resultados/command%d_output.txt", contador);

        FILE *out = fopen(caminho_saida, "w");
        if (!out)
        {
            perror("Erro a criar ficheiro de output");
            contador++;
            continue;
        }

        // Execute query
        executar_query(gestor, linha, out);

        fclose(out);
        contador++;
    }

    fclose(input);
}