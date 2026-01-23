#define _GNU_SOURCE
#include "parsers/parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_COLUNAS 100
#define MMAP_MIN_SIZE (16 * 1024 * 1024)
#define IO_BUFFER_SIZE (8 * 1024 * 1024)

typedef struct {
    void *data;
    size_t size;
} parser_mmap_buffer_t;

static GPtrArray *g_mmap_buffers = NULL;
static int g_mmap_em_uso = 0;
static int g_mmap_decidido = 0;
static int g_mmap_ativo = 0;
static int g_sem_erros_ativo = 0;

static int parser_mmap_ativado(void)
{
    if (!g_mmap_decidido) {
        const char *env = getenv("LI3_USE_MMAP");
        g_mmap_ativo = (env && (*env == '1' || *env == 'y' || *env == 'Y'));
        g_mmap_decidido = 1;
    }
    return g_mmap_ativo;
}

static void parser_mmap_cleanup(void)
{
    if (!g_mmap_buffers)
        return;

    for (guint i = 0; i < g_mmap_buffers->len; i++) {
        parser_mmap_buffer_t *buf = g_ptr_array_index(g_mmap_buffers, i);
        if (buf && buf->data && buf->size > 0)
            munmap(buf->data, buf->size);
    }

    g_ptr_array_free(g_mmap_buffers, TRUE);
    g_mmap_buffers = NULL;
}

static void parser_mmap_registar(void *data, size_t size)
{
    if (!g_mmap_buffers) {
        g_mmap_buffers = g_ptr_array_new_with_free_func(g_free);
        atexit(parser_mmap_cleanup);
    }

    parser_mmap_buffer_t *buf = g_new(parser_mmap_buffer_t, 1);
    buf->data = data;
    buf->size = size;
    g_ptr_array_add(g_mmap_buffers, buf);
}

int parser_mmap_em_uso(void)
{
    return g_mmap_em_uso;
}

int parser_sem_erros_ativo(void)
{
    return g_sem_erros_ativo;
}

void parser_definir_sem_erros(int ativo)
{
    g_sem_erros_ativo = ativo ? 1 : 0;
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
                return numColunas;
            }
        }
    }

    if (numColunas < colunas_necessarias && numColunas < max_colunas)
        colunas[numColunas++] = campo_inicio;

    colunas[numColunas] = NULL;
    return numColunas;
}

static int parser_carrega_mmap(void *contexto, const char *ficheiro_csv,
                               AdicionaObjeto adiciona_objeto, LinhaParaObjeto linha_para_objeto,
                               DestroiObjeto destroi_objeto, int max_colunas,
                               int colunas_necessarias)
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

    parser_mmap_registar(data, size);
    g_mmap_em_uso = 1;

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
            char *colunas[MAX_COLUNAS + 1];
            int numColunas = parser_dividir_csv_ate(cur, colunas, max_colunas, colunas_necessarias);
            if (numColunas >= colunas_necessarias) {
                gpointer objeto = linha_para_objeto(colunas);
                if (objeto) {
                    if (!adiciona_objeto(contexto, objeto))
                        destroi_objeto(objeto);
                }
            }
        }

        if (nl == end)
            break;
        cur = nl + 1;
    }

    close(fd);
    g_mmap_em_uso = 0;
    return 1;
}

/**
 * @brief Carrega um ficheiro CSV e processa cada linha.
 */
void parser_carrega(void *contexto, const char *ficheiro_csv, AdicionaObjeto adiciona_objeto,
                    LinhaParaObjeto linha_para_objeto, DestroiObjeto destroi_objeto,
                    int max_colunas, int colunas_necessarias)
{
    int sem_erros = (ficheiro_csv && strstr(ficheiro_csv, "sem_erros") != NULL);
    if (colunas_necessarias <= 0 || colunas_necessarias > max_colunas)
        colunas_necessarias = max_colunas;

    g_sem_erros_ativo = sem_erros;
    if (sem_erros && parser_mmap_ativado()) {
        struct stat st;
        if (stat(ficheiro_csv, &st) == 0 && st.st_size > MMAP_MIN_SIZE) {
            if (parser_carrega_mmap(contexto, ficheiro_csv, adiciona_objeto, linha_para_objeto,
                                    destroi_objeto, max_colunas, colunas_necessarias)) {
                g_sem_erros_ativo = 0;
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
    if (!sem_erros) {
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
            fprintf(ficheiro_erros, "%s", linha);
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

        char *colunas[MAX_COLUNAS + 1];
        int numColunas =
            parser_dividir_csv_ate(linha_trabalho, colunas, max_colunas, colunas_necessarias);
        if (numColunas < colunas_necessarias) {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%s", linha);
            continue;
        }

        gpointer objeto = linha_para_objeto(colunas);
        if (objeto == NULL) {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%s", linha);
        } else if (adiciona_objeto(contexto, objeto)) {
            // ok
        } else {
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
    g_sem_erros_ativo = 0;
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
