#define _GNU_SOURCE
#include "parsers/parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_COLUNAS 100
#define IO_BUFFER_SIZE (8 * 1024 * 1024)
static int g_sem_erros_ativo = 0;
static int g_dataset_grande_ativo = 0;
static int g_skip_error_log_decidido = 0;
static int g_skip_error_log = 0;

static int parser_skip_error_log(void)
{
    if (!g_skip_error_log_decidido) {
        const char *env = getenv("LI3_SKIP_ERROR_LOG");
        if (env) {
            g_skip_error_log = (*env == '1' || *env == 'y' || *env == 'Y');
        } else {
            g_skip_error_log = g_dataset_grande_ativo;
        }
        g_skip_error_log_decidido = 1;
    }
    return g_skip_error_log;
}

int parser_sem_erros_ativo(void)
{
    return g_sem_erros_ativo;
}

int parser_mmap_em_uso(void)
{
    return 0;
}

void parser_definir_sem_erros(int ativo)
{
    g_sem_erros_ativo = ativo ? 1 : 0;
}

int parser_dataset_grande_ativo(void)
{
    return g_dataset_grande_ativo;
}

void parser_definir_dataset_grande(int ativo)
{
    g_dataset_grande_ativo = ativo ? 1 : 0;
}

/**
 * @brief Divide uma linha CSV em colunas, respeitando aspas.
 */
int parser_dividir_csv(char *linha, char **colunas, int max_colunas)
{
    return parser_dividir_csv_ate(linha, colunas, max_colunas, max_colunas);
}

int parser_dividir_csv_ate(char *linha, char **colunas, int max_colunas, int colunas_necessarias)
{
    if (!linha || !colunas || max_colunas <= 0)
        return 0;

    if (colunas_necessarias <= 0 || colunas_necessarias > max_colunas)
        colunas_necessarias = max_colunas;

    int numColunas = 0;
    char *campo_inicio = linha;
    int dentro_aspas = 0;

    for (char *p = linha; *p; p++) {
        if (*p == '"')
            dentro_aspas = !dentro_aspas;
        else if (*p == ',' && !dentro_aspas) {
            *p = '\0';
            colunas[numColunas++] = campo_inicio;
            campo_inicio = p + 1;
            if (numColunas == colunas_necessarias) {
                colunas[numColunas] = NULL;
                for (int i = numColunas + 1; i <= max_colunas; i++)
                    colunas[i] = NULL;
                return numColunas;
            }
        }
    }

    if (numColunas < colunas_necessarias && numColunas < max_colunas)
        colunas[numColunas++] = campo_inicio;

    colunas[numColunas] = NULL;
    return numColunas;
}

static void parser_tratar_linha(char *linha, char *linha_parse, size_t tamanho_parse, size_t len,
                                int sem_erros, int skip_errors, FILE *ficheiro_erros,
                                void *contexto, AdicionaObjeto adiciona_objeto,
                                LinhaParaObjeto linha_para_objeto, DestroiObjeto destroi_objeto,
                                int max_colunas, int colunas_necessarias)
{
    char *linha_trabalho = linha;
    size_t len_parse = len;

    if (!sem_erros && !skip_errors) {
        if (len + 1 > tamanho_parse)
            return;
        memcpy(linha_parse, linha, len);
        linha_parse[len] = '\0';
        if (len > 0 && linha_parse[len - 1] == '\r')
            linha_parse[len - 1] = '\0';
        linha_trabalho = linha_parse;
        len_parse = strlen(linha_trabalho);
    } else {
        if (len > 0 && linha[len - 1] == '\r')
            linha[len - 1] = '\0';
    }

    if (len_parse == 0)
        return;

    char *colunas[MAX_COLUNAS + 1];
    int numColunas =
        parser_dividir_csv_ate(linha_trabalho, colunas, max_colunas, colunas_necessarias);
    if (numColunas < colunas_necessarias) {
        if (ficheiro_erros)
            fprintf(ficheiro_erros, "%s\n", linha);
        return;
    }

    gpointer objeto = linha_para_objeto(colunas);
    if (objeto == NULL) {
        if (ficheiro_erros)
            fprintf(ficheiro_erros, "%s\n", linha);
        return;
    }

    if (!adiciona_objeto(contexto, objeto)) {
        if (ficheiro_erros)
            fprintf(ficheiro_erros, "%s\n", linha);
        destroi_objeto(objeto);
    }
}

static void parser_carrega_streaming(void *contexto, const char *ficheiro_csv,
                                     AdicionaObjeto adiciona_objeto,
                                     LinhaParaObjeto linha_para_objeto,
                                     DestroiObjeto destroi_objeto, int max_colunas,
                                     int colunas_necessarias, int sem_erros)
{
    int fd = open(ficheiro_csv, O_RDONLY);
    if (fd < 0) {
        perror("Erro ao abrir ficheiro CSV");
        return;
    }
    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);

    int skip_errors = parser_skip_error_log();
    FILE *ficheiro_erros = NULL;
    if (!sem_erros && !skip_errors) {
        char *nome_base = utils_obtem_nome_ficheiro(ficheiro_csv);
        char *caminho_erros = g_strdup_printf("resultados/%s_errors.csv", nome_base);
        g_free(nome_base);

        ficheiro_erros = fopen(caminho_erros, "w");
        g_free(caminho_erros);
        if (ficheiro_erros)
            setvbuf(ficheiro_erros, NULL, _IOFBF, IO_BUFFER_SIZE);
    }

    char *buffer = malloc(IO_BUFFER_SIZE + 1);
    if (!buffer) {
        if (ficheiro_erros)
            fclose(ficheiro_erros);
        close(fd);
        return;
    }

    char *linha_parse = NULL;
    size_t tamanho_parse = 0;

    ssize_t lidos;
    size_t buffer_len = 0;
    int primeira_linha = 1;

    while ((lidos = read(fd, buffer + buffer_len, IO_BUFFER_SIZE - buffer_len - 1)) > 0) {
        size_t total = buffer_len + (size_t)lidos;
        size_t start = 0;
        buffer[total] = '\0';

        while (start < total) {
            char *nl = memchr(buffer + start, '\n', total - start);
            if (!nl)
                break;

            size_t len = (size_t)(nl - (buffer + start));
            char *linha = buffer + start;
            char saved = *nl;
            *nl = '\0';

            if (primeira_linha) {
                if (ficheiro_erros)
                    fprintf(ficheiro_erros, "%.*s\n", (int)len, linha);
                primeira_linha = 0;
            } else {
                if (!sem_erros && !skip_errors) {
                    if (len + 1 > tamanho_parse) {
                        char *novo = realloc(linha_parse, len + 1);
                        if (!novo)
                            goto cleanup;
                        linha_parse = novo;
                        tamanho_parse = len + 1;
                    }
                }
                parser_tratar_linha(linha, linha_parse, tamanho_parse, len, sem_erros,
                                    skip_errors, ficheiro_erros, contexto, adiciona_objeto,
                                    linha_para_objeto, destroi_objeto, max_colunas,
                                    colunas_necessarias);
            }

            *nl = saved;
            start = (size_t)(nl - buffer) + 1;
        }

        buffer_len = total - start;
        if (buffer_len > 0)
            memmove(buffer, buffer + start, buffer_len);
    }

    if (lidos >= 0 && buffer_len > 0) {
        char *linha = buffer;
        size_t len = buffer_len;
        buffer[buffer_len] = '\0';
        if (primeira_linha) {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%.*s\n", (int)len, linha);
        } else {
            if (!sem_erros && !skip_errors) {
                if (len + 1 > tamanho_parse) {
                    char *novo = realloc(linha_parse, len + 1);
                    if (!novo)
                        goto cleanup;
                    linha_parse = novo;
                    tamanho_parse = len + 1;
                }
            }
            parser_tratar_linha(linha, linha_parse, tamanho_parse, len, sem_erros, skip_errors,
                                ficheiro_erros, contexto, adiciona_objeto, linha_para_objeto,
                                destroi_objeto, max_colunas, colunas_necessarias);
        }
    }

cleanup:
    free(linha_parse);
    free(buffer);
    if (ficheiro_erros)
        fclose(ficheiro_erros);
    close(fd);
}

/**
 * @brief Carrega um ficheiro CSV e processa cada linha.
 */
void parser_carrega(void *contexto, const char *ficheiro_csv, AdicionaObjeto adiciona_objeto,
                    LinhaParaObjeto linha_para_objeto, DestroiObjeto destroi_objeto,
                    int max_colunas, int colunas_necessarias)
{
    int sem_erros = (ficheiro_csv && strstr(ficheiro_csv, "sem_erros") != NULL);
    int dataset_grande = (ficheiro_csv && strstr(ficheiro_csv, "grande") != NULL);
    if (colunas_necessarias <= 0 || colunas_necessarias > max_colunas)
        colunas_necessarias = max_colunas;

    g_sem_erros_ativo = sem_erros;
    g_dataset_grande_ativo = dataset_grande;
    parser_carrega_streaming(contexto, ficheiro_csv, adiciona_objeto, linha_para_objeto,
                             destroi_objeto, max_colunas, colunas_necessarias, sem_erros);
    g_sem_erros_ativo = 0;
    g_dataset_grande_ativo = 0;
}

/**
 * @brief Converte uma string datetime no formato "YYYY-MM-DD HH:MM" para time_t.
 */
time_t parser_datetime_para_time(const char *datetime)
{
    if (!datetime)
        return (time_t)-1;

    int y, m, d, hh, mm;

    if (sscanf(datetime, "%d-%d-%d %d:%d", &y, &m, &d, &hh, &mm) != 5)
        return (time_t)-1;

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
