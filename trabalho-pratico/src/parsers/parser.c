#define _GNU_SOURCE
#include "parsers/parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <time.h>

#define MAX_COLUNAS 100

// ═══════════════════════════════════════════════════
// FUNÇÃO PARA REMOVER ASPAS DOS CAMPOS CSV
// ═══════════════════════════════════════════════════
static void remove_aspas(char *str)
{
    if (!str)
        return;

    size_t len = strlen(str);

    // Se string começa e termina com aspas, remove-as
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"')
    {
        str[len - 1] = '\0';            // Remove aspa do fim
        memmove(str, str + 1, len - 1); // Move tudo 1 posição à esquerda
    }
}

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

    char *nome_base = utils_obtem_nome_ficheiro(ficheiro_csv);
    char *caminho_erros = g_strdup_printf("resultados/%s_errors.csv", nome_base);
    g_free(nome_base);

    FILE *ficheiro_erros = fopen(caminho_erros, "w");
    g_free(caminho_erros);

    char *linha = NULL;
    size_t tamanho = 0;
    ssize_t lidos;

    if ((lidos = getline(&linha, &tamanho, ficheiro)) != -1)
    {
        if (ficheiro_erros)
            fprintf(ficheiro_erros, "%s", linha);
    }

    while ((lidos = getline(&linha, &tamanho, ficheiro)) != -1)
    {
        char *linha_original = g_strdup(linha);
        char *colunas[MAX_COLUNAS + 1];
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
                if (numColunas >= MAX_COLUNAS)
                    break;
            }
        }

        if (numColunas < MAX_COLUNAS)
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

        for (int i = 0; i < numColunas; i++)
            remove_aspas(colunas[i]);

        gpointer objeto = linha_para_objeto(colunas);

        if (objeto == NULL)
        {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%s", linha_original);
        }
        else if (adiciona_objeto(contexto, objeto))
        {
        }
        else
        {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%s", linha_original);
            destroi_objeto(objeto);
        }

        g_free(linha_original);
    }

    free(linha);
    fclose(ficheiro);
    if (ficheiro_erros)
        fclose(ficheiro_erros);
}
time_t parser_datetime_para_time(const char *datetime)
{
    if (!datetime)
        return (time_t)-1;

    int y, m, d, hh, mm;

    // APENAS hífens (REMOVER suporte a barras!)
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
