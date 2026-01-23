#include "gestores/gestor_voos.h"
#include "estruturas/voo_table.h"
#include "gestores/gestor_avioes.h"
#include "gestores/gestor_aeroportos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_voos.h"
#include "entidades/voos.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

/**
 * @struct gestor_voos
 * @brief Estrutura que mantém todos os voos e estatísticas.
 */
struct gestor_voos
{
    voo_table_t tabela;           /**< Tabela de voos (key -> voo_t*) */
    int **q3_contagens;           /**< Contagens para query 3 (origem_idx -> array) */
    int q3_min_day;               /**< Menor dia de voo processado */
    int q3_max_day;               /**< Maior dia de voo processado */
    int q3_range;                 /**< Range de dias processados */
    GHashTable *atrasos_airline;  /**< Atrasos acumulados por companhia aérea */
    GArray *q5_cache;             /**< Cache para query 5 (média de atrasos por airline) */
};

/**
 * @struct atraso_airline_t
 * @brief Estatísticas de atraso por companhia aérea.
 */
typedef struct
{
    guint count;        
    double total_delay; 
} atraso_airline_t;

typedef struct
{
    gestor_voos_t *gestor;
} q3_ctx_t;

typedef struct
{
    gestor_aeroportos_t *gestor_aeroportos;
} contagens_ctx_t;

/**
 * @brief Arredonda um valor de atraso para milissegundos.
 */
static long long q5_round_millis(double v)
{
    if (v >= 0.0)
        return (long long)(v * 1000.0 + 0.5);
    return (long long)(v * 1000.0 - 0.5);
}

/**
 * @brief Função de comparação para ordenar o cache de atrasos.
 */
static gint q5_cache_cmp(gconstpointer a, gconstpointer b)
{
    const gestor_voos_q5_t *ra = a;
    const gestor_voos_q5_t *rb = b;

    long long ra_ms = q5_round_millis(ra->avg_delay);
    long long rb_ms = q5_round_millis(rb->avg_delay);
    if (ra_ms > rb_ms)
        return -1;
    if (ra_ms < rb_ms)
        return 1;

    return strcmp(ra->airline, rb->airline);
}

static void q3_visit_voo(voo_t *voo, void *ud)
{
    q3_ctx_t *ctx = ud;
    gestor_voos_t *g = ctx->gestor;
    if (voo_obter_status_codigo(voo) == 2)
        return;
    int act_day = voo_obter_actual_departure_dia(voo);
    if (act_day < 0)
        return;

    int orig_idx = voo_obter_origin_idx(voo);
    if (orig_idx < 0)
        return;

    int idx = act_day - g->q3_min_day;
    if (idx < 0 || idx >= g->q3_range)
        return;

    int *counts = g->q3_contagens[orig_idx];
    if (!counts)
    {
        counts = g_malloc0(sizeof(int) * g->q3_range);
        g->q3_contagens[orig_idx] = counts;
    }
    counts[idx]++;
}

static void contagens_visit_voo(voo_t *voo, void *ud)
{
    contagens_ctx_t *ctx = ud;
    gestor_aeroportos_t *ga = ctx->gestor_aeroportos;
    if (voo_obter_status_codigo(voo) == 2)
        return;

    int passageiros = voo_obter_passageiros(voo);
    if (passageiros <= 0)
        return;

    int orig_idx = voo_obter_origin_idx(voo);
    int dest_idx = voo_obter_destination_idx(voo);

    if (orig_idx >= 0)
    {
        aeroporto_t *a = gestor_aeroportos_obter_por_idx(ga, orig_idx);
        if (a)
            aeroporto_incrementar_partidas(a, passageiros);
    }
    if (dest_idx >= 0)
    {
        aeroporto_t *a = gestor_aeroportos_obter_por_idx(ga, dest_idx);
        if (a)
            aeroporto_incrementar_chegadas(a, passageiros);
    }
}

/**
 * @brief Liberta a memória do cache de query 5.
 */
static void q5_cache_destruir(GArray *cache)
{
    if (!cache)
        return;

    for (guint i = 0; i < cache->len; i++)
    {
        gestor_voos_q5_t *item = &g_array_index(cache, gestor_voos_q5_t, i);
        g_free(item->airline);
    }

    g_array_free(cache, TRUE);
}


/**
 * @brief Cria e inicializa um gestor de voos.
 * @return Ponteiro para gestor_voos_t criado.
 */
gestor_voos_t *gestor_voos_criar(void)
{
    gestor_voos_t *g = malloc(sizeof(gestor_voos_t));
    voo_table_init(&g->tabela, 1 << 16);
    g->q3_contagens = NULL;
    g->q3_min_day = INT_MAX;
    g->q3_max_day = INT_MIN;
    g->q3_range = 0;
    g->atrasos_airline = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    g->q5_cache = NULL;
    return g;
}

/**
 * @brief Liberta toda a memória associada a um gestor de voos.
 */
void gestor_voos_destruir(gestor_voos_t *gestor)
{
    if (!gestor)
        return;
    if (gestor->q3_contagens)
    {
        for (int i = 0; i < (26 * 26 * 26); i++)
            g_free(gestor->q3_contagens[i]);
        g_free(gestor->q3_contagens);
        gestor->q3_contagens = NULL;
    }
    if (gestor->q5_cache)
        q5_cache_destruir(gestor->q5_cache);
    g_hash_table_destroy(gestor->atrasos_airline);
    voo_table_destroy(&gestor->tabela, voo_destruir);
    voo_intern_pool_destruir();
    free(gestor);
}

/**
 * @brief Adiciona um voo ao gestor, atualizando estatísticas.
 */
void gestor_voos_adicionar(gestor_voos_t *gestor, voo_info_t *info)
{
    if (!gestor || !info)
        return;

    uint64_t key = info->key;
    if (key == 0)
    {
        voo_info_destruir(info);
        return;
    }
    uint64_t keyplus = key + 1ull;
    if (voo_table_lookup(&gestor->tabela, keyplus))
    {
        voo_info_destruir(info);
        return;
    }

    int status = info->status;
    int act_day = info->act_dep_day;
    if (status != 2 && act_day >= 0)
    {
        if (act_day < gestor->q3_min_day)
            gestor->q3_min_day = act_day;
        if (act_day > gestor->q3_max_day)
            gestor->q3_max_day = act_day;
    }

    if (status == 1)
    {
        const char *airline = info->airline;
        double atraso = (info->atraso_min >= 0) ? (double)info->atraso_min : -1.0;
        if (airline && *airline && atraso >= 0.0)
        {
            atraso_airline_t *stats = g_hash_table_lookup(gestor->atrasos_airline, airline);
            if (!stats)
            {
                stats = g_new0(atraso_airline_t, 1);
                g_hash_table_insert(gestor->atrasos_airline, g_strdup(airline), stats);
            }
            stats->count++;
            stats->total_delay += atraso;
        }
    }

    voo_t *voo = voo_criar_from_info(info);
    if (!voo)
    {
        voo_info_destruir(info);
        return;
    }

    if (!voo_table_insert(&gestor->tabela, keyplus, voo))
    {
        voo_destruir(voo);
        voo_info_destruir(info);
        return;
    }

    voo_info_destruir(info);
}

/**
 * @brief Obtém um voo pelo ID.
 */
voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id)
{
    uint64_t key = 0;
    if (!gestor || !flight_id || !utils_flight_id_key(flight_id, &key))
        return NULL;
    return gestor_voos_obter_por_key(gestor, key);
}

voo_t *gestor_voos_obter_por_key(gestor_voos_t *gestor, uint64_t key)
{
    if (!gestor)
        return NULL;
    return voo_table_lookup(&gestor->tabela, (uint64_t)(key + 1ull));
}


/**
 * @brief Retorna o número de voos no gestor.
 */
unsigned gestor_voos_contar(const gestor_voos_t *gestor)
{
    return gestor ? (unsigned)voo_table_size(&gestor->tabela) : 0;
}

/**
 * @brief Executa uma função para cada voo do gestor.
 */
void gestor_voos_para_cada(gestor_voos_t *gestor, void (*func)(voo_t *, void *), void *user_data)
{
    if (!gestor || !func)
        return;

    voo_table_foreach(&gestor->tabela, func, user_data);
}

/**
 * @brief Callback interno para adicionar voos.
 */
static gboolean _adiciona_voo_callback(void *contexto, void *objeto)
{
    gestor_voos_adicionar(contexto, objeto);
    return TRUE;
}

/**
 * @struct contexto_voos_validacao_t
 * @brief Contexto usado ao carregar voos com validação.
 */
typedef struct
{
    gestor_voos_t *gestor_voos; 
    gestor_avioes_t *gestor_avioes; 
} contexto_voos_validacao_t;

/**
 * @brief Callback interno para adicionar voos validados.
 */
static gboolean _adiciona_voo_validado(void *contexto, void *objeto)
{
    contexto_voos_validacao_t *ctx = contexto;
    voo_info_t *info = objeto;

    const char *aircraft = info ? info->aircraft : NULL;
    if (!aircraft || !ctx->gestor_avioes)
        return FALSE;

    aviao_t *aviao = gestor_avioes_obter_por_id(ctx->gestor_avioes, aircraft);
    if (!aviao)
        return FALSE;

    if (info->status != 2)
        aviao_incrementar_contagem_voos(aviao, 1);

    gestor_voos_adicionar(ctx->gestor_voos, info);
    return TRUE;
}

/**
 * @brief Carrega voos de um CSV sem validação extra.
 */
void gestor_voos_carregar(gestor_voos_t *gestor, const char *ficheiro_csv)
{
    if (gestor && ficheiro_csv)
        parser_carrega(gestor, ficheiro_csv, _adiciona_voo_callback, (LinhaParaObjeto)valida_voo, (DestroiObjeto)voo_info_destruir, 12, 12);
}

/**
 * @brief Carrega voos de um CSV com validação de aeronaves.
 */
void gestor_voos_carregar_com_validacao(gestor_voos_t *gestor, const char *ficheiro_csv, gestor_avioes_t *gestor_avioes)
{
    if (gestor && ficheiro_csv)
    {
        contexto_voos_validacao_t ctx = {
            .gestor_voos = gestor,
            .gestor_avioes = gestor_avioes};
        parser_carrega(&ctx, ficheiro_csv, _adiciona_voo_validado, (LinhaParaObjeto)valida_voo, (DestroiObjeto)voo_info_destruir, 12, 12);
    }
}

/**
 * @brief Prepara estruturas de contagem para Q3.
 */
void gestor_voos_preparar_q3(gestor_voos_t *gestor)
{
    if (!gestor || gestor->q3_contagens)
        return;

    if (gestor->q3_min_day == INT_MAX || gestor->q3_max_day == INT_MIN)
        return;

    gestor->q3_range = gestor->q3_max_day - gestor->q3_min_day + 1;
    if (gestor->q3_range <= 0)
        return;

    gestor->q3_contagens = g_malloc0(sizeof(int *) * (26 * 26 * 26));

    q3_ctx_t ctx = {.gestor = gestor};
    voo_table_foreach(&gestor->tabela, q3_visit_voo, &ctx);

    for (int i = 0; i < (26 * 26 * 26); i++)
    {
        int *counts = gestor->q3_contagens[i];
        if (!counts)
            continue;
        for (int j = 1; j < gestor->q3_range; j++)
            counts[j] += counts[j - 1];
    }
}

/**
 * @brief Calcula a melhor origem de voos em um intervalo de dias.
 */
gboolean gestor_voos_melhor_origem_intervalo(gestor_voos_t *gestor, int dia_inicio, int dia_fim, const char **out_origem, guint *out_contagem)
{
    if (!gestor || !out_origem || !out_contagem)
        return FALSE;

    if (!gestor->q3_contagens)
        gestor_voos_preparar_q3(gestor);

    if (!gestor->q3_contagens || gestor->q3_range <= 0)
        return FALSE;

    if (dia_fim < gestor->q3_min_day || dia_inicio > gestor->q3_max_day)
        return FALSE;

    if (dia_inicio < gestor->q3_min_day)
        dia_inicio = gestor->q3_min_day;
    if (dia_fim > gestor->q3_max_day)
        dia_fim = gestor->q3_max_day;

    int start_idx = dia_inicio - gestor->q3_min_day;
    int end_idx = dia_fim - gestor->q3_min_day;

    int melhor_idx = -1;
    guint melhor_contagem = 0;

    for (int idx = 0; idx < (26 * 26 * 26); idx++)
    {
        int *counts = gestor->q3_contagens[idx];
        if (!counts)
            continue;
        int total = counts[end_idx] - (start_idx > 0 ? counts[start_idx - 1] : 0);
        if (total <= 0)
            continue;

        if ((guint)total > melhor_contagem ||
            ((guint)total == melhor_contagem && (melhor_idx < 0 || idx < melhor_idx)))
        {
            melhor_contagem = (guint)total;
            melhor_idx = idx;
        }
    }

    if (melhor_idx < 0)
        return FALSE;

    *out_origem = utils_aeroporto_codigo_const(melhor_idx);
    if (!*out_origem)
        return FALSE;
    *out_contagem = melhor_contagem;
    return TRUE;
}

/**
 * @brief Executa um callback para cada atraso por companhia aérea.
 */
void gestor_voos_para_cada_atraso(gestor_voos_t *gestor, void (*callback)(const char *airline, guint count, double total_delay, void *), void *user_data)
{
    if (!gestor || !callback)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->atrasos_airline);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char *airline = key;
        atraso_airline_t *stats = value;
        callback(airline, stats->count, stats->total_delay, user_data);
    }
}

/**
 * @brief Retorna o cache de Q5 (média de atrasos por companhia aérea), gerando se necessário.
 */
const GArray *gestor_voos_obter_q5_cache(gestor_voos_t *gestor)
{
    if (!gestor)
        return NULL;

    if (gestor->q5_cache)
        return gestor->q5_cache;

    GArray *cache = g_array_new(FALSE, FALSE, sizeof(gestor_voos_q5_t));

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->atrasos_airline);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char *airline = key;
        atraso_airline_t *stats = value;
        if (!airline || !*airline || !stats || stats->count == 0)
            continue;

        gestor_voos_q5_t item = {
            .airline = g_strdup(airline),
            .count = stats->count,
            .avg_delay = stats->total_delay / stats->count};
        g_array_append_val(cache, item);
    }

    g_array_sort(cache, q5_cache_cmp);
    gestor->q5_cache = cache;
    return gestor->q5_cache;
}

/**
 * @brief Atualiza contagens de chegadas e partidas nos aeroportos.
 */
void gestor_voos_atualizar_contagens_aeroportos(gestor_voos_t *gestor_voos, gestor_aeroportos_t *gestor_aeroportos)
{
    if (!gestor_voos || !gestor_aeroportos)
        return;

    contagens_ctx_t ctx = {.gestor_aeroportos = gestor_aeroportos};
    voo_table_foreach(&gestor_voos->tabela, contagens_visit_voo, &ctx);
}
