#define _GNU_SOURCE
#include "parsers/parser_reservas.h"
#include "parsers/parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_COLUNAS_RESERVAS 16
#define RESERVA_COLS 8
#define MMAP_MIN_SIZE (16 * 1024 * 1024)
#define IO_BUFFER_SIZE (8 * 1024 * 1024)

static int g_mmap_decidido = 0;
static int g_mmap_ativo = 0;
static int g_skip_error_log_decidido = 0;
static int g_skip_error_log = 0;

static int parser_mmap_ativado(void)
{
    if (!g_mmap_decidido) {
        const char *env_res = getenv("LI3_USE_MMAP_RESERVAS");
        const char *env = env_res ? env_res : getenv("LI3_USE_MMAP");
        g_mmap_ativo = (env && (*env == '1' || *env == 'y' || *env == 'Y'));
        g_mmap_decidido = 1;
    }
    return g_mmap_ativo;
}

static int parser_skip_error_log(void)
{
    if (!g_skip_error_log_decidido) {
        const char *env = getenv("LI3_SKIP_ERROR_LOG");
        g_skip_error_log = (env && (*env == '1' || *env == 'y' || *env == 'Y'));
        g_skip_error_log_decidido = 1;
    }
    return g_skip_error_log;
}

static int carregar_mmap(const char *ficheiro_csv, void *contexto,
                         ReservaProcessaLinha processa_linha)
{
    int fd = open(ficheiro_csv, O_RDONLY);
    if (fd < 0)
        return 0;

    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);

    struct stat st;
    if (fstat(fd, &st) != 0 || st.st_size <= 0) {
        close(fd);
        return 0;
    }

    size_t size = (size_t)st.st_size;
    char *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return 0;
    }

    madvise(data, size, MADV_SEQUENTIAL);

    char *cur = data;
    char *end = data + size;

    char *nl = memchr(cur, '\n', (size_t)(end - cur));
    if (!nl) {
        munmap(data, size);
        close(fd);
        return 0;
    }

    *nl = '\0';
    cur = nl + 1;

    while (cur < end) {
        nl = memchr(cur, '\n', (size_t)(end - cur));
        if (!nl)
            nl = end;

        if (nl > cur && nl[-1] == '\r')
            nl[-1] = '\0';
        if (nl < end)
            *nl = '\0';

        if (*cur) {
            char *colunas[MAX_COLUNAS_RESERVAS + 1];
            int numColunas =
                parser_dividir_csv_ate(cur, colunas, MAX_COLUNAS_RESERVAS, RESERVA_COLS);
            if (numColunas >= RESERVA_COLS)
                processa_linha(contexto, colunas);
        }

        if (nl == end)
            break;
        cur = nl + 1;
    }

    munmap(data, size);
    close(fd);
    return 1;
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
    if (sem_erros && parser_mmap_ativado()) {
        struct stat st;
        if (stat(ficheiro_csv, &st) == 0 && st.st_size > MMAP_MIN_SIZE) {
            if (carregar_mmap(ficheiro_csv, contexto, processa_linha)) {
                parser_definir_sem_erros(0);
                parser_definir_dataset_grande(0);
                return;
            }
        }
    }

    FILE *ficheiro = fopen(ficheiro_csv, "r");
    if (!ficheiro) {
        perror("Erro ao abrir ficheiro CSV");
        return;
    }
    setvbuf(ficheiro, NULL, _IOFBF, IO_BUFFER_SIZE);

    FILE *ficheiro_erros = NULL;
    if (!sem_erros && !parser_skip_error_log()) {
        char *nome_base = utils_obtem_nome_ficheiro(ficheiro_csv);
        char *caminho_erros = g_strdup_printf("resultados/%s_errors.csv", nome_base);
        g_free(nome_base);

        ficheiro_erros = fopen(caminho_erros, "w");
        g_free(caminho_erros);
        if (ficheiro_erros)
            setvbuf(ficheiro_erros, NULL, _IOFBF, IO_BUFFER_SIZE);
    }

    char *linha = NULL;
    size_t tamanho = 0;
    char *linha_parse = NULL;
    size_t tamanho_parse = 0;
    ssize_t lidos;

    if ((lidos = getline(&linha, &tamanho, ficheiro)) != -1) {
        if (ficheiro_erros)
            fputs(linha, ficheiro_erros);
    }

    while ((lidos = getline(&linha, &tamanho, ficheiro)) != -1) {
        char *linha_trabalho = linha;
        if (!sem_erros) {
            size_t necessario = (size_t)lidos + 1;
            if (necessario > tamanho_parse) {
                char *novo = realloc(linha_parse, necessario);
                if (!novo)
                    break;
                linha_parse = novo;
                tamanho_parse = necessario;
            }
            memcpy(linha_parse, linha, necessario);
            linha_trabalho = linha_parse;
        }

        char *colunas[MAX_COLUNAS_RESERVAS + 1];
        int numColunas =
            parser_dividir_csv_ate(linha_trabalho, colunas, MAX_COLUNAS_RESERVAS, RESERVA_COLS);
        if (numColunas < RESERVA_COLS) {
            if (ficheiro_erros)
                fputs(linha, ficheiro_erros);
            continue;
        }

        if (!processa_linha(contexto, colunas)) {
            if (ficheiro_erros)
                fputs(linha, ficheiro_erros);
        }
    }

    free(linha_parse);
    free(linha);
    fclose(ficheiro);
    if (ficheiro_erros)
        fclose(ficheiro_erros);
    parser_definir_sem_erros(0);
    parser_definir_dataset_grande(0);
}
