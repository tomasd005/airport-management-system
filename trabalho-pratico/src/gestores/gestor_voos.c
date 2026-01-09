#include "gestores/gestor_voos.h"
#include "gestores/gestor_avioes.h"
#include "gestores/gestor_aeroportos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_voos.h"
#include "entidades/voos.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

/**
 * @struct gestor_voos
 * @brief Estrutura que mantém todos os voos e estatísticas.
 */
struct gestor_voos
{
    GHashTable *tabela;           /**< Tabela de voos (id -> voo_t*) */
    GHashTable *q3_contagens;     /**< Contagens para query 3 (origem -> array de contagens por dia) */
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
 * @brief Callback para destruir contagens de origem de Q3.
 */
static void contagens_origem_destruir(gpointer data)
{
    g_free(data);
}

/**
 * @brief Cria e inicializa um gestor de voos.
 * @return Ponteiro para gestor_voos_t criado.
 */
gestor_voos_t *gestor_voos_criar(void)
{
    gestor_voos_t *g = malloc(sizeof(gestor_voos_t));
    g->tabela = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)voo_destruir);
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
        g_hash_table_destroy(gestor->q3_contagens);
    if (gestor->q5_cache)
        q5_cache_destruir(gestor->q5_cache);
    g_hash_table_destroy(gestor->atrasos_airline);
    g_hash_table_destroy(gestor->tabela);
    voo_intern_pool_destruir();
    free(gestor);
}

/**
 * @brief Adiciona um voo ao gestor, atualizando estatísticas.
 */
void gestor_voos_adicionar(gestor_voos_t *gestor, voo_t *voo)
{
    if (!gestor || !voo)
        return;

    const char *id = voo_obter_id(voo);
    if (!id || g_hash_table_contains(gestor->tabela, id))
    {
        voo_destruir(voo);
        return;
    }

    int status = voo_obter_status_codigo(voo);
    int act_day = voo_obter_actual_departure_dia(voo);
    if (status != 2 && act_day >= 0)
    {
        if (act_day < gestor->q3_min_day)
            gestor->q3_min_day = act_day;
        if (act_day > gestor->q3_max_day)
            gestor->q3_max_day = act_day;
    }

    if (status == 1)
    {
        const char *airline = voo_obter_airline(voo);
        double atraso = voo_calcular_atraso_minutos(voo);
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

    g_hash_table_insert(gestor->tabela, (gpointer)id, voo);
    voo_descartar_aircraft(voo);
    voo_descartar_airline(voo);
}

/**
 * @brief Obtém um voo pelo ID.
 */
voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id)
{
    return (gestor && flight_id) ? g_hash_table_lookup(gestor->tabela, flight_id) : NULL;
}

/**
 * @brief Retorna a tabela interna de voos.
 */
GHashTable *gestor_voos_obter_tabela(gestor_voos_t *gestor)
{
    return gestor ? gestor->tabela : NULL;
}

/**
 * @brief Retorna o número de voos no gestor.
 */
unsigned gestor_voos_contar(const gestor_voos_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->tabela) : 0;
}

/**
 * @brief Executa uma função para cada voo do gestor.
 */
void gestor_voos_para_cada(gestor_voos_t *gestor, void (*func)(voo_t *, void *), void *user_data)
{
    if (!gestor || !func)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
        func(value, user_data);
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
    voo_t *voo = objeto;

    const char *aircraft = voo_obter_aircraft(voo);
    if (!aircraft || !ctx->gestor_avioes)
        return FALSE;

    aviao_t *aviao = gestor_avioes_obter_por_id(ctx->gestor_avioes, aircraft);
    if (!aviao)
        return FALSE;

    if (voo_obter_status_codigo(voo) != 2)
        aviao_incrementar_contagem_voos(aviao, 1);

    gestor_voos_adicionar(ctx->gestor_voos, voo);
    return TRUE;
}

/**
 * @brief Carrega voos de um CSV sem validação extra.
 */
void gestor_voos_carregar(gestor_voos_t *gestor, const char *ficheiro_csv)
{
    if (gestor && ficheiro_csv)
        parser_carrega(gestor, ficheiro_csv, _adiciona_voo_callback, (LinhaParaObjeto)valida_voo, (DestroiObjeto)voo_destruir);
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
        parser_carrega(&ctx, ficheiro_csv, _adiciona_voo_validado, (LinhaParaObjeto)valida_voo, (DestroiObjeto)voo_destruir);
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

    gestor->q3_contagens = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, contagens_origem_destruir);

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        voo_t *voo = value;
        if (voo_obter_status_codigo(voo) == 2)
            continue;
        int act_day = voo_obter_actual_departure_dia(voo);
        if (act_day < 0)
            continue;

        const char *origin = voo_obter_origin(voo);
        if (!origin)
            continue;

        int idx = act_day - gestor->q3_min_day;
        if (idx < 0 || idx >= gestor->q3_range)
            continue;

        int *counts = g_hash_table_lookup(gestor->q3_contagens, origin);
        if (!counts)
        {
            counts = g_malloc0(sizeof(int) * gestor->q3_range);
            g_hash_table_insert(gestor->q3_contagens, (gpointer)origin, counts);
        }
        counts[idx]++;
    }

    GHashTableIter iter_counts;
    gpointer ckey, cval;
    g_hash_table_iter_init(&iter_counts, gestor->q3_contagens);

    while (g_hash_table_iter_next(&iter_counts, &ckey, &cval))
    {
        int *counts = cval;
        for (int i = 1; i < gestor->q3_range; i++)
            counts[i] += counts[i - 1];
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

    const char *melhor_origem = NULL;
    guint melhor_contagem = 0;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->q3_contagens);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char *origin = key;
        int *counts = value;
        int total = counts[end_idx] - (start_idx > 0 ? counts[start_idx - 1] : 0);
        if (total <= 0)
            continue;

        if ((guint)total > melhor_contagem ||
            ((guint)total == melhor_contagem && (!melhor_origem || strcmp(origin, melhor_origem) < 0)))
        {
            melhor_contagem = (guint)total;
            melhor_origem = origin;
        }
    }

    if (!melhor_origem)
        return FALSE;

    *out_origem = melhor_origem;
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

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor_voos->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        voo_t *voo = value;
        if (voo_obter_status_codigo(voo) == 2)
            continue;

        int passageiros = voo_obter_passageiros(voo);
        if (passageiros <= 0)
            continue;

        const char *orig = voo_obter_origin(voo);
        const char *dest = voo_obter_destination(voo);

        if (orig)
        {
            aeroporto_t *a = gestor_aeroportos_obter_por_codigo(gestor_aeroportos, orig);
            if (a)
                aeroporto_incrementar_partidas(a, passageiros);
        }
        if (dest)
        {
            aeroporto_t *a = gestor_aeroportos_obter_por_codigo(gestor_aeroportos, dest);
            if (a)
                aeroporto_incrementar_chegadas(a, passageiros);
        }
    }
}
