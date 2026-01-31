#define _GNU_SOURCE
#include "parsers/parser_reservas.h"
#include "parsers/parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_COLUNAS_RESERVAS 16
#define RESERVA_COLS 8
#define IO_BUFFER_SIZE (8 * 1024 * 1024)
static int g_skip_error_log_decidido = 0;
static int g_skip_error_log = 0;

static int parser_skip_error_log(void)
{
    if (!g_skip_error_log_decidido) {
        const char *env = getenv("LI3_SKIP_ERROR_LOG");
        if (env) {
            g_skip_error_log = (*env == '1' || *env == 'y' || *env == 'Y');
        } else {
            g_skip_error_log = parser_dataset_grande_ativo();
        }
        g_skip_error_log_decidido = 1;
    }
    return g_skip_error_log;
}

static void parser_reservas_tratar_linha(char *linha, char *linha_parse, size_t tamanho_parse,
                                         size_t len, int sem_erros, int skip_errors,
                                         FILE *ficheiro_erros, void *contexto,
                                         ReservaProcessaLinha processa_linha)
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

    char *colunas[MAX_COLUNAS_RESERVAS + 1];
    int numColunas =
        parser_dividir_csv_ate(linha_trabalho, colunas, MAX_COLUNAS_RESERVAS, RESERVA_COLS);
    if (numColunas < RESERVA_COLS) {
        if (ficheiro_erros)
            fprintf(ficheiro_erros, "%s\n", linha);
        return;
    }

    if (!processa_linha(contexto, colunas)) {
        if (ficheiro_erros)
            fprintf(ficheiro_erros, "%s\n", linha);
    }
}

static void parser_reservas_streaming(void *contexto, const char *ficheiro_csv,
                                      ReservaProcessaLinha processa_linha, int sem_erros)
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
    size_t buffer_len = 0;
    ssize_t lidos;
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
                parser_reservas_tratar_linha(linha, linha_parse, tamanho_parse, len, sem_erros,
                                             skip_errors, ficheiro_erros, contexto, processa_linha);
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
            parser_reservas_tratar_linha(linha, linha_parse, tamanho_parse, len, sem_erros,
                                         skip_errors, ficheiro_erros, contexto, processa_linha);
        }
    }

cleanup:
    free(linha_parse);
    free(buffer);
    if (ficheiro_erros)
        fclose(ficheiro_erros);
    close(fd);
}

void parser_reservas_carregar(void *contexto, const char *ficheiro_csv,
                              ReservaProcessaLinha processa_linha)
{
    if (!ficheiro_csv || !processa_linha)
        return;

    int sem_erros = (strstr(ficheiro_csv, "sem_erros") != NULL);
    int dataset_grande = (strstr(ficheiro_csv, "grande") != NULL);
    parser_definir_sem_erros(sem_erros);
    parser_definir_dataset_grande(dataset_grande);
    parser_reservas_streaming(contexto, ficheiro_csv, processa_linha, sem_erros);
    parser_definir_sem_erros(0);
    parser_definir_dataset_grande(0);
}
