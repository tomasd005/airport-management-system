#include "parsers/parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_COLUNAS 100
#define IO_BUFFER_SIZE (8 * 1024 * 1024)
#define PARSE_QUEUE_CAP 128

typedef struct {
    char *data;
    size_t len;
} parse_chunk_t;

typedef struct {
    parse_chunk_t items[PARSE_QUEUE_CAP];
    size_t head;
    size_t tail;
    size_t count;
    int done;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} parse_queue_t;

typedef struct {
    parse_queue_t *queue;
    void *contexto;
    AdicionaObjeto adiciona_objeto;
    LinhaParaObjeto linha_para_objeto;
    DestroiObjeto destroi_objeto;
    int max_colunas;
    int colunas_necessarias;
    pthread_mutex_t *add_mutex;
} parse_worker_ctx_t;
static __thread int g_sem_erros_ativo = 0;
static __thread int g_dataset_grande_ativo = 0;
static __thread int g_mmap_ativo = 0;
static __thread int g_skip_error_log_decidido = 0;
static __thread int g_skip_error_log = 0;

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

static int parser_threads_ativado(void)
{
    const char *env = getenv("LI3_PARSE_THREADS");
    const char *unsafe = getenv("LI3_PARSE_UNSAFE");
    if (!unsafe || (*unsafe != '1' && *unsafe != 'y' && *unsafe != 'Y'))
        return 0;
    if (!env || !*env)
        return 0;
    return atoi(env);
}

static void parse_queue_init(parse_queue_t *q)
{
    memset(q, 0, sizeof(*q));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

static void parse_queue_destroy(parse_queue_t *q)
{
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
}

static void parse_queue_push(parse_queue_t *q, parse_chunk_t *task)
{
    pthread_mutex_lock(&q->mutex);
    while (q->count == PARSE_QUEUE_CAP)
        pthread_cond_wait(&q->not_full, &q->mutex);
    q->items[q->tail] = *task;
    q->tail = (q->tail + 1) % PARSE_QUEUE_CAP;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

static int parse_queue_pop(parse_queue_t *q, parse_chunk_t *out)
{
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0 && !q->done)
        pthread_cond_wait(&q->not_empty, &q->mutex);
    if (q->count == 0 && q->done) {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    *out = q->items[q->head];
    q->head = (q->head + 1) % PARSE_QUEUE_CAP;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    return 1;
}

static void parse_queue_finish(parse_queue_t *q)
{
    pthread_mutex_lock(&q->mutex);
    q->done = 1;
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

static void *parse_worker(void *arg)
{
    parse_worker_ctx_t *ctx = (parse_worker_ctx_t *)arg;
    parse_chunk_t task;

    while (parse_queue_pop(ctx->queue, &task)) {
        char *cur = task.data;
        char *end = task.data + task.len;

        while (cur < end) {
            char *nl = memchr(cur, '\n', (size_t)(end - cur));
            if (!nl)
                nl = end;

            if (nl > cur && nl[-1] == '\r')
                nl[-1] = '\0';
            if (nl < end)
                *nl = '\0';

            if (*cur) {
                char *colunas[MAX_COLUNAS + 1];
                int numColunas = parser_dividir_csv_ate(cur, colunas, ctx->max_colunas,
                                                       ctx->colunas_necessarias);
                if (numColunas >= ctx->colunas_necessarias) {
                    gpointer objeto = ctx->linha_para_objeto(colunas);
                    if (objeto) {
                        int ok;
                        pthread_mutex_lock(ctx->add_mutex);
                        ok = ctx->adiciona_objeto(ctx->contexto, objeto);
                        pthread_mutex_unlock(ctx->add_mutex);
                        if (!ok)
                            ctx->destroi_objeto(objeto);
                    }
                }
            }

            if (nl == end)
                break;
            cur = nl + 1;
        }

        free(task.data);
    }

    return NULL;
}

int parser_sem_erros_ativo(void)
{
    return g_sem_erros_ativo;
}

int parser_mmap_em_uso(void)
{
    return g_mmap_ativo;
}

void parser_definir_mmap_em_uso(int ativo)
{
    g_mmap_ativo = ativo ? 1 : 0;
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
        if (len >= tamanho_parse)
            return;
        memcpy(linha_parse, linha, len);
        linha_parse[len] = '\0';
        if (len > 0 && linha_parse[len - 1] == '\r') {
            linha_parse[len - 1] = '\0';
            len_parse = len - 1;
        } else {
            len_parse = len;
        }
        linha_trabalho = linha_parse;
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

static void parser_carrega_streaming_paralelo(void *contexto, const char *ficheiro_csv,
                                              AdicionaObjeto adiciona_objeto,
                                              LinhaParaObjeto linha_para_objeto,
                                              DestroiObjeto destroi_objeto, int max_colunas,
                                              int colunas_necessarias, int sem_erros, int n_threads)
{
    int fd = open(ficheiro_csv, O_RDONLY);
    if (fd < 0) {
        perror("Erro ao abrir ficheiro CSV");
        return;
    }
    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);

    char *buffer = malloc(IO_BUFFER_SIZE + 1);
    if (!buffer) {
        close(fd);
        return;
    }

    parse_queue_t queue;
    parse_queue_init(&queue);

    pthread_mutex_t add_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t *threads = calloc((size_t)n_threads, sizeof(pthread_t));
    parse_worker_ctx_t ctx = {
        .queue = &queue,
        .contexto = contexto,
        .adiciona_objeto = adiciona_objeto,
        .linha_para_objeto = linha_para_objeto,
        .destroi_objeto = destroi_objeto,
        .max_colunas = max_colunas,
        .colunas_necessarias = colunas_necessarias,
        .add_mutex = &add_mutex,
    };

    for (int i = 0; i < n_threads; i++)
        pthread_create(&threads[i], NULL, parse_worker, &ctx);

    size_t buffer_len = 0;
    ssize_t lidos;
    int primeira_linha = 1;

    while ((lidos = read(fd, buffer + buffer_len, IO_BUFFER_SIZE - buffer_len - 1)) > 0) {
        size_t total = buffer_len + (size_t)lidos;
        buffer[total] = '\0';

        char *last_nl = memrchr(buffer, '\n', total);
        if (!last_nl) {
            buffer_len = total;
            if (buffer_len >= IO_BUFFER_SIZE - 1) {
                buffer[total] = '\0';
                parse_chunk_t task = {.data = strdup(buffer), .len = strlen(buffer)};
                if (task.data) {
                    if (primeira_linha) {
                        char *nl = strchr(task.data, '\n');
                        if (nl) {
                            size_t header_len = (size_t)(nl - task.data);
                            memmove(task.data, nl + 1, task.len - header_len - 1);
                            task.len -= header_len + 1;
                        }
                        primeira_linha = 0;
                    }
                    if (task.len > 0)
                        parse_queue_push(&queue, &task);
                    else
                        free(task.data);
                }
                buffer_len = 0;
            }
            continue;
        }

        size_t chunk_len = (size_t)(last_nl - buffer);
        parse_chunk_t task = {0};
        task.data = malloc(chunk_len + 1);
        if (task.data) {
            memcpy(task.data, buffer, chunk_len);
            task.data[chunk_len] = '\0';
            task.len = chunk_len;

            if (primeira_linha) {
                char *nl = strchr(task.data, '\n');
                if (nl) {
                    size_t header_len = (size_t)(nl - task.data);
                    memmove(task.data, nl + 1, task.len - header_len - 1);
                    task.len -= header_len + 1;
                }
                primeira_linha = 0;
            }

            if (task.len > 0)
                parse_queue_push(&queue, &task);
            else
                free(task.data);
        }

        buffer_len = total - (chunk_len + 1);
        if (buffer_len > 0)
            memmove(buffer, last_nl + 1, buffer_len);
    }

    if (lidos >= 0 && buffer_len > 0) {
        buffer[buffer_len] = '\0';
        parse_chunk_t task = {.data = strdup(buffer), .len = strlen(buffer)};
        if (task.data) {
            if (primeira_linha) {
                char *nl = strchr(task.data, '\n');
                if (nl) {
                    size_t header_len = (size_t)(nl - task.data);
                    memmove(task.data, nl + 1, task.len - header_len - 1);
                    task.len -= header_len + 1;
                }
                primeira_linha = 0;
            }
            if (task.len > 0)
                parse_queue_push(&queue, &task);
            else
                free(task.data);
        }
    }

    parse_queue_finish(&queue);
    for (int i = 0; i < n_threads; i++)
        pthread_join(threads[i], NULL);

    free(threads);
    parse_queue_destroy(&queue);
    free(buffer);
    close(fd);
}

typedef struct {
    const char *start;
    const char *end;
    void *contexto;
    AdicionaObjeto adiciona_objeto;
    LinhaParaObjeto linha_para_objeto;
    DestroiObjeto destroi_objeto;
    int max_colunas;
    int colunas_necessarias;
    pthread_mutex_t *add_mutex;
} mmap_worker_ctx_t;

static void *mmap_worker(void *arg) __attribute__((unused));
static void *mmap_worker(void *arg)
{
    mmap_worker_ctx_t *ctx = (mmap_worker_ctx_t *)arg;
    const char *cur = ctx->start;
    const char *end = ctx->end;
    char linebuf[8192];
    char *colunas[MAX_COLUNAS + 1];

    while (cur < end) {
        const char *nl = memchr(cur, '\n', (size_t)(end - cur));
        if (!nl)
            nl = end;

        size_t len = (size_t)(nl - cur);
        if (len > 0 && cur[len - 1] == '\r')
            len--;

        if (len > 0 && len < sizeof(linebuf)) {
            memcpy(linebuf, cur, len);
            linebuf[len] = '\0';

            int numColunas = parser_dividir_csv_ate(linebuf, colunas, ctx->max_colunas,
                                                     ctx->colunas_necessarias);
            if (numColunas >= ctx->colunas_necessarias) {
                gpointer objeto = ctx->linha_para_objeto(colunas);
                if (objeto) {
                    int ok;
                    pthread_mutex_lock(ctx->add_mutex);
                    ok = ctx->adiciona_objeto(ctx->contexto, objeto);
                    pthread_mutex_unlock(ctx->add_mutex);
                    if (!ok)
                        ctx->destroi_objeto(objeto);
                }
            }
        }

        if (nl == end)
            break;
        cur = nl + 1;
    }
    return NULL;
}

/**
 * @brief Carrega um ficheiro via mmap (read-only) — skip error log.
 */
static void parser_carrega_mmap(void *contexto, const char *ficheiro_csv,
                                AdicionaObjeto adiciona_objeto, LinhaParaObjeto linha_para_objeto,
                                DestroiObjeto destroi_objeto, int max_colunas,
                                int colunas_necessarias) __attribute__((unused));
static void parser_carrega_mmap(void *contexto, const char *ficheiro_csv,
                                AdicionaObjeto adiciona_objeto, LinhaParaObjeto linha_para_objeto,
                                DestroiObjeto destroi_objeto, int max_colunas,
                                int colunas_necessarias)
{
    int fd = open(ficheiro_csv, O_RDONLY);
    if (fd < 0)
        return;

    struct stat st;
    if (fstat(fd, &st) < 0 || st.st_size == 0) {
        close(fd);
        return;
    }

    size_t file_size = (size_t)st.st_size;
    char *map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (map == MAP_FAILED)
        return;

    madvise(map, file_size, MADV_SEQUENTIAL | MADV_WILLNEED);

    /* Skip header line */
    const char *data_start = memchr(map, '\n', file_size);
    if (!data_start) {
        munmap(map, file_size);
        return;
    }
    data_start++;

    const char *data_end = map + file_size;

    const char *cur = data_start;
    char linebuf[8192];
    char *colunas[MAX_COLUNAS + 1];

    while (cur < data_end) {
        const char *nl = memchr(cur, '\n', (size_t)(data_end - cur));
        if (!nl)
            nl = data_end;

        size_t len = (size_t)(nl - cur);
        if (len > 0 && cur[len - 1] == '\r')
            len--;

        if (len > 0 && len < sizeof(linebuf)) {
            memcpy(linebuf, cur, len);
            linebuf[len] = '\0';

            int numColunas =
                parser_dividir_csv_ate(linebuf, colunas, max_colunas, colunas_necessarias);
            if (numColunas >= colunas_necessarias) {
                gpointer objeto = linha_para_objeto(colunas);
                if (objeto) {
                    if (!adiciona_objeto(contexto, objeto))
                        destroi_objeto(objeto);
                }
            }
        }

        if (nl == data_end)
            break;
        cur = nl + 1;
    }

    munmap(map, file_size);
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
    g_skip_error_log_decidido = 0;
    g_skip_error_log = 0;
    int n_threads = parser_threads_ativado();
    int skip_errors = parser_skip_error_log();
    const char *use_mmap_env = getenv("LI3_USE_MMAP");
    int use_mmap = use_mmap_env && (*use_mmap_env == '1' || *use_mmap_env == 'y' ||
                                    *use_mmap_env == 'Y');

    if (use_mmap && (sem_erros || skip_errors)) {
        g_mmap_ativo = 1;
        parser_carrega_mmap(contexto, ficheiro_csv, adiciona_objeto, linha_para_objeto,
                            destroi_objeto, max_colunas, colunas_necessarias);
        g_mmap_ativo = 0;
    } else if (n_threads > 1 && (sem_erros || skip_errors)) {
        parser_carrega_streaming_paralelo(contexto, ficheiro_csv, adiciona_objeto,
                                          linha_para_objeto, destroi_objeto, max_colunas,
                                          colunas_necessarias, sem_erros, n_threads);
    } else {
        parser_carrega_streaming(contexto, ficheiro_csv, adiciona_objeto, linha_para_objeto,
                                 destroi_objeto, max_colunas, colunas_necessarias, sem_erros);
    }
    g_sem_erros_ativo = 0;
    g_dataset_grande_ativo = 0;
    g_mmap_ativo = 0;
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
