#include "gestor_principal.h"
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

/**
 * @struct GestorPrincipal
 * @brief Estrutura principal do modo batch.
 *
 * Contém todos os gestores de dados necessários para a execução
 * das queries a partir de um ficheiro de input.
 */
struct GestorPrincipal
{
    gestor_aeroportos_t *aeroportos;
    gestor_avioes_t *avioes;
    gestor_voos_t *voos;
    gestor_passageiros_t *passageiros;
    gestor_reservas_t *reservas;
};

/**
 * @brief Cria e inicializa o gestor principal.
 *
 * @return Ponteiro para o gestor criado ou NULL em caso de erro
 */
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

/**
 * @brief Liberta toda a memória associada ao gestor principal.
 *
 * @param gestor Ponteiro para o gestor principal
 */
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

/**
 * @brief Remove espaços em branco no início e fim de uma string.
 *
 * @param str String a limpar
 */
static void trim_string(char *str)
{
    if (!str)
        return;

    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end))
        *end-- = '\0';

    char *start = str;
    while (*start && isspace((unsigned char)*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);
}

/**
 * @brief Carrega todos os ficheiros CSV do dataset.
 *
 * @param gestor Gestor principal
 * @param pasta Caminho para a pasta do dataset
 */
static void carregar_dados(gestor_principal_t *gestor, const char *pasta)
{
    char caminho[512];

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
}

/**
 * @brief Executa uma única query a partir de uma linha de input.
 *
 * A função identifica o tipo da query e encaminha para a função
 * correspondente.
 *
 * @param gestor Gestor principal
 * @param linha_completa Linha completa da query
 * @param output Ficheiro de saída
 */
static void executar_query(
    gestor_principal_t *gestor,
    const char *linha_completa,
    FILE *output)
{
    char linha[256];
    strncpy(linha, linha_completa, sizeof(linha) - 1);
    linha[sizeof(linha) - 1] = '\0';

    char *p = linha;
    while (*p && isspace((unsigned char)*p))
        p++;

    if (!*p)
    {
        fprintf(output, "\n");
        return;
    }

    char *tipo_str = p;
    while (*p && !isspace((unsigned char)*p))
        p++;
    if (*p)
        *p++ = '\0';

    int tipo = atoi(tipo_str);

    switch (tipo)
    {
    case 1:
        while (*p && isspace((unsigned char)*p))
            p++;
        trim_string(p);
        query1(gestor->aeroportos, gestor->voos, gestor->reservas,
               linha_completa, p, output);
        break;

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
        trim_string(p);

        query2(gestor->avioes, gestor->voos, N,
               strlen(p) ? p : NULL, linha_completa, output);
        break;
    }

    case 3:
    {
        while (*p && isspace((unsigned char)*p))
            p++;
        char *inicio = p;
        while (*p && !isspace((unsigned char)*p))
            p++;
        if (*p)
            *p++ = '\0';
        while (*p && isspace((unsigned char)*p))
            p++;
        trim_string(p);

        if (*inicio && *p)
            query3(gestor->aeroportos, gestor->voos,
                   inicio, p, linha_completa, output);
        else
            fprintf(output, "\n");
        break;
    }

    case 4:
    {
        while (*p && isspace((unsigned char)*p))
            p++;
        char *inicio = NULL, *fim = NULL;

        if (*p)
        {
            inicio = p;
            while (*p && !isspace((unsigned char)*p))
                p++;
            if (*p)
                *p++ = '\0';
            while (*p && isspace((unsigned char)*p))
                p++;
            if (*p)
                fim = p;
        }

        if (fim)
            trim_string(fim);

        query4(gestor->reservas, gestor->voos, gestor->passageiros,
               inicio, fim, linha_completa, output);
        break;
    }

    case 5:
        while (*p && isspace((unsigned char)*p))
            p++;
        trim_string(p);
        query5(gestor->voos, atoi(p), linha_completa, output);
        break;

    case 6:
        while (*p && isspace((unsigned char)*p))
            p++;
        trim_string(p);
        if (*p)
            query6(gestor->reservas, gestor->voos, gestor->passageiros,
                   p, linha_completa, output);
        else
            fprintf(output, "\n");
        break;

    default:
        fprintf(output, "\n");
        break;
    }
}

/**
 * @brief Executa o modo batch do programa.
 *
 * Carrega os dados, lê o ficheiro de queries e gera um ficheiro
 * de output por comando.
 *
 * @param gestor Gestor principal
 * @param pasta_dados Caminho para o dataset
 * @param ficheiro_input Caminho para o ficheiro de queries
 */
void gestor_principal_executar(
    gestor_principal_t *gestor,
    const char *pasta_dados,
    const char *ficheiro_input)
{
    if (!gestor || !pasta_dados || !ficheiro_input)
        return;

    carregar_dados(gestor, pasta_dados);

    FILE *input = fopen(ficheiro_input, "r");
    if (!input)
        return;

    char linha[256];
    int contador = 1;

    while (fgets(linha, sizeof(linha), input))
    {
        size_t len = strlen(linha);
        while (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r'))
            linha[--len] = '\0';

        if (len == 0)
            continue;

        char caminho_saida[256];
        snprintf(caminho_saida, sizeof(caminho_saida),
                 "resultados/command%d_output.txt", contador);

        FILE *out = fopen(caminho_saida, "w");
        if (!out)
        {
            contador++;
            continue;
        }

        executar_query(gestor, linha, out);
        fclose(out);
        contador++;
    }

    fclose(input);
}
