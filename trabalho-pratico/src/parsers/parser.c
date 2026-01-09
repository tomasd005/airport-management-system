#define _GNU_SOURCE
#include "parsers/parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <time.h>

#define MAX_COLUNAS 100

/**
 * @brief Divide uma linha CSV em colunas, respeitando aspas.
 *
 * Esta função trata campos entre aspas corretamente e substitui
 * vírgulas por '\0' para delimitar strings.
 *
 * @param linha Linha do CSV a ser dividida.
 * @param colunas Vetor de ponteiros onde cada coluna será armazenada.
 * @param max_colunas Número máximo de colunas a processar.
 * @return Número de colunas obtidas.
 */
int parser_dividir_csv(char *linha, char **colunas, int max_colunas)
{
    if (!linha || !colunas || max_colunas <= 0)
        return 0;

    int numColunas = 0;
    char *campo_inicio = linha;
    int dentro_aspas = 0;

    for (char *p = linha; *p; p++)
    {
        if (*p == '"')
            dentro_aspas = !dentro_aspas;
        else if (*p == ',' && !dentro_aspas)
        {
            *p = '\0';
            colunas[numColunas++] = campo_inicio;
            campo_inicio = p + 1;
            if (numColunas >= max_colunas)
                break;
        }
    }

    if (numColunas < max_colunas)
    {
        char *nl = strchr(campo_inicio, '\n');
        if (nl)
            *nl = '\0';
        nl = strchr(campo_inicio, '\r');
        if (nl)
            *nl = '\0';
        colunas[numColunas++] = campo_inicio;
    }
    colunas[numColunas] = NULL;
    return numColunas;
}

/**
 * @brief Carrega um ficheiro CSV e processa cada linha.
 *
 * Lê linha a linha, converte para objeto com `linha_para_objeto`,
 * adiciona ao contexto com `adiciona_objeto` e grava erros.
 *
 * @param contexto Contexto onde os objetos serão adicionados.
 * @param ficheiro_csv Caminho para o ficheiro CSV.
 * @param adiciona_objeto Função que adiciona um objeto ao contexto.
 * @param linha_para_objeto Função que converte uma linha em objeto.
 * @param destroi_objeto Função que destrói o objeto se necessário.
 */
void parser_carrega(void *contexto,
                    const char *ficheiro_csv,
                    AdicionaObjeto adiciona_objeto,
                    LinhaParaObjeto linha_para_objeto,
                    DestroiObjeto destroi_objeto)
{
    FILE *ficheiro = fopen(ficheiro_csv, "r");
    if (!ficheiro)
    {
        perror("Erro ao abrir ficheiro CSV");
        return;
    }
    setvbuf(ficheiro, NULL, _IOFBF, 1 << 20);

    char *nome_base = utils_obtem_nome_ficheiro(ficheiro_csv);
    char *caminho_erros = g_strdup_printf("resultados/%s_errors.csv", nome_base);
    g_free(nome_base);

    FILE *ficheiro_erros = fopen(caminho_erros, "w");
    g_free(caminho_erros);
    if (ficheiro_erros)
        setvbuf(ficheiro_erros, NULL, _IOFBF, 1 << 20);

    char *linha = NULL;
    size_t tamanho = 0;
    char *linha_parse = NULL;
    size_t tamanho_parse = 0;
    ssize_t lidos;

    // Copia cabeçalho para ficheiro de erros
    if ((lidos = getline(&linha, &tamanho, ficheiro)) != -1)
    {
        if (ficheiro_erros)
            fprintf(ficheiro_erros, "%s", linha);
    }

    while ((lidos = getline(&linha, &tamanho, ficheiro)) != -1)
    {
        size_t necessario = (size_t)lidos + 1;
        if (necessario > tamanho_parse)
        {
            char *novo = realloc(linha_parse, necessario);
            if (!novo)
                break;
            linha_parse = novo;
            tamanho_parse = necessario;
        }
        memcpy(linha_parse, linha, necessario);

        char *colunas[MAX_COLUNAS + 1];
        int numColunas = parser_dividir_csv(linha_parse, colunas, MAX_COLUNAS);
        if (numColunas <= 0)
        {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%s", linha);
            continue;
        }

        gpointer objeto = linha_para_objeto(colunas);

        if (objeto == NULL)
        {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%s", linha);
        }
        else if (adiciona_objeto(contexto, objeto))
        {
            // Objeto adicionado com sucesso
        }
        else
        {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%s", linha);
            destroi_objeto(objeto);
        }
    }

    free(linha_parse);
    free(linha);
    fclose(ficheiro);
    if (ficheiro_erros)
        fclose(ficheiro_erros);
}

/**
 * @brief Converte uma string datetime no formato "YYYY-MM-DD HH:MM" para time_t.
 *
 * Suporta apenas hífens como separadores de data.
 *
 * @param datetime String no formato "YYYY-MM-DD HH:MM"
 * @return time_t equivalente ou -1 em caso de erro.
 */
time_t parser_datetime_para_time(const char *datetime)
{
    if (!datetime)
        return (time_t)-1;

    int y, m, d, hh, mm;

    // Apenas hífens suportados
    if (sscanf(datetime, "%d-%d-%d %d:%d", &y, &m, &d, &hh, &mm) != 5)
        return (time_t)-1;

    // Validações
    if (m < 1 || m > 12)
        return (time_t)-1;
    if (d < 1 || d > 31)
        return (time_t)-1;
    if (hh < 0 || hh > 23)
        return (time_t)-1;
    if (mm < 0 || mm > 59)
        return (time_t)-1;

    struct tm tm = {0};
    tm.tm_year = y - 1900;
    tm.tm_mon = m - 1;
    tm.tm_mday = d;
    tm.tm_hour = hh;
    tm.tm_min = mm;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;

    return mktime(&tm);
}
