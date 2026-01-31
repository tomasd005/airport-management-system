#include "gestores/gestor_reservas.h"
#include "gestores/gestor_passageiros.h"
#include "gestores/gestor_voos.h"
#include "parsers/parser_reservas.h"
#include "parsers/parser.h"
#include "estruturas/doc_total_table.h"
#include "validacoes/validacao_reservas.h"
#include "entidades/passageiros.h"
#include "entidades/voos.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

/**
 * @struct melhor_destino_t
 * @brief Armazena o destino mais frequente para uma nacionalidade.
 */
typedef struct {
    char destino[4];
    guint count;
} melhor_destino_t;

/**
 * @struct gasto_t
 * @brief Estrutura para armazenar o gasto total de um passageiro.
 */
typedef struct {
    uint32_t doc_key;
    double total;
} gasto_t;

#define NUM_AEROPORTOS (26 * 26 * 26)

typedef struct {
    gestor_reservas_t *gestor;
    gestor_voos_t *gestor_voos;
    gestor_passageiros_t *gestor_passageiros;
} reservas_ctx_t;

static void acumular_passageiros_voos_ptr(voo_t *const *voos, size_t num_voos);

static inline gpointer doc_key_para_ptr(uint32_t key)
{
    return GUINT_TO_POINTER((guint)(key + 1));
}

static inline uint64_t flight_id_key_fast(const char *id)
{
    if (!id || !isupper((unsigned char)id[0]) || !isupper((unsigned char)id[1]))
        return 0;

    if (id[2] == '\0')
        return 0;

    uint32_t letters = (uint32_t)(id[0] - 'A') * 26u + (uint32_t)(id[1] - 'A');
    uint32_t num = 0;
    int digits = 0;
    const char *p = id + 2;
    while (*p) {
        if (!isdigit((unsigned char)*p))
            return 0;
        num = num * 10u + (uint32_t)(*p - '0');
        p++;
        digits++;
        if (digits > 7)
            return 0;
    }
    if (digits < 4)
        return 0;
    return ((uint64_t)letters * 100000000ull) + ((uint64_t)digits * 10000000ull) + num;
}

static int reserva_id_key(const char *id, uint32_t *out_key)
{
    if (!id || !out_key)
        return 0;
    if (id[0] != 'R')
        return 0;
    if (strlen(id) != 10)
        return 0;

    uint32_t value = 0;
    for (int i = 1; i < 10; i++) {
        unsigned char c = (unsigned char)id[i];
        if (c < '0' || c > '9')
            return 0;
        value = value * 10u + (uint32_t)(c - '0');
    }

    *out_key = value;
    return 1;
}

/**
 * @struct gestor_reservas
 * @brief Estrutura principal para gerenciar reservas.
 */
struct gestor_reservas {
    guint total_reservas;
    GHashTable *gastos_por_semana;
    GHashTable *top10_por_semana;
    GHashTable *destinos_por_nacionalidade;
    GHashTable *melhor_dest_por_nacionalidade;
    uint8_t *reservas_ids_bits;
    size_t reservas_ids_cap;
    int cache_semana;
    doc_total_table_t *cache_gastos;
};

static gboolean reservas_ids_marcar(gestor_reservas_t *gestor, uint32_t id)
{
    if (!gestor)
        return FALSE;

    size_t idx = (size_t)id;
    size_t needed = idx + 1;
    if (needed > gestor->reservas_ids_cap) {
        size_t new_cap = gestor->reservas_ids_cap ? gestor->reservas_ids_cap : 1024;
        while (new_cap < needed)
            new_cap <<= 1;

        size_t old_bytes = (gestor->reservas_ids_cap + 7) / 8;
        size_t new_bytes = (new_cap + 7) / 8;
        uint8_t *novo = realloc(gestor->reservas_ids_bits, new_bytes);
        if (!novo)
            return FALSE;
        if (new_bytes > old_bytes)
            memset(novo + old_bytes, 0, new_bytes - old_bytes);
        gestor->reservas_ids_bits = novo;
        gestor->reservas_ids_cap = new_cap;
    }

    size_t byte = idx >> 3;
    uint8_t mask = (uint8_t)(1u << (idx & 7u));
    if (gestor->reservas_ids_bits[byte] & mask)
        return FALSE;

    gestor->reservas_ids_bits[byte] |= mask;
    return TRUE;
}

/**
 * @brief Cria um array de contagens por destino.
 * @return Novo array de contagens.
 */
static uint32_t *criar_contagem_destinos(void)
{
    return g_malloc0(sizeof(uint32_t) * NUM_AEROPORTOS);
}

/**
 * @brief Cria e inicializa um gestor de reservas.
 * @return Ponteiro para gestor_reservas_t criado.
 */
gestor_reservas_t *gestor_reservas_criar(void)
{
    gestor_reservas_t *g = malloc(sizeof(*g));
    g->total_reservas = 0;
    g->gastos_por_semana = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                                                 (GDestroyNotify)doc_total_table_destroy);
    g->top10_por_semana = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                                                (GDestroyNotify)g_ptr_array_unref);
    g->destinos_por_nacionalidade = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
    g->melhor_dest_por_nacionalidade = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
    g->reservas_ids_bits = NULL;
    g->reservas_ids_cap = 0;
    g->cache_semana = -1;
    g->cache_gastos = NULL;
    return g;
}

/**
 * @brief Liberta toda a memória associada a um gestor de reservas.
 * @param gestor Ponteiro para gestor_reservas_t.
 */
void gestor_reservas_destruir(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;

    if (gestor->gastos_por_semana)
        g_hash_table_destroy(gestor->gastos_por_semana);
    if (gestor->top10_por_semana)
        g_hash_table_destroy(gestor->top10_por_semana);
    if (gestor->destinos_por_nacionalidade)
        g_hash_table_destroy(gestor->destinos_por_nacionalidade);
    if (gestor->melhor_dest_por_nacionalidade)
        g_hash_table_destroy(gestor->melhor_dest_por_nacionalidade);
    free(gestor->reservas_ids_bits);
    free(gestor);
}

/**
 * @brief Retorna o número total de reservas processadas.
 * @param gestor Ponteiro para gestor_reservas_t.
 * @return Total de reservas.
 */
unsigned int gestor_reservas_numero(gestor_reservas_t *gestor)
{
    return gestor ? gestor->total_reservas : 0;
}

/**
 * @brief Acumula gastos de um passageiro para a semana de um voo.
 */
static void acumular_gastos_semana(gestor_reservas_t *gestor, uint32_t doc_key, double preco,
                                   voo_t *const *voos, size_t num_voos)
{
    if (!gestor || !voos || num_voos == 0)
        return;

    voo_t *voo = voos[0];
    if (!voo)
        return;

    int semana = voo_obter_semana(voo);
    if (semana < 0)
        return;

    doc_total_table_t *gastos = NULL;
    if (gestor->cache_semana == semana && gestor->cache_gastos) {
        gastos = gestor->cache_gastos;
    } else {
        gastos = g_hash_table_lookup(gestor->gastos_por_semana, GINT_TO_POINTER(semana));
        if (!gastos) {
            gastos = doc_total_table_create(1024);
            if (!gastos)
                return;
            g_hash_table_insert(gestor->gastos_por_semana, GINT_TO_POINTER(semana), gastos);
        }
        gestor->cache_semana = semana;
        gestor->cache_gastos = gastos;
    }

    doc_total_table_add(gastos, doc_key, preco);
}

/**
 * @brief Acumula destinos visitados por nacionalidade.
 */
static void acumular_destinos_nacionalidade(gestor_reservas_t *gestor, passageiro_t *passageiro,
                                            voo_t *const *voos, size_t num_voos)
{
    if (!gestor || !passageiro || !voos || num_voos == 0)
        return;

    const char *nac = passageiro_obter_nacionalidade(passageiro);
    if (!nac || !*nac)
        return;

    uint32_t *destinos = passageiro_obter_destinos_counts(passageiro);
    if (!destinos) {
        destinos = g_hash_table_lookup(gestor->destinos_por_nacionalidade, nac);
        if (!destinos) {
            destinos = criar_contagem_destinos();
            g_hash_table_insert(gestor->destinos_por_nacionalidade, (gpointer)nac, destinos);
        }
        passageiro_definir_destinos_counts(passageiro, destinos);
    }

    for (size_t i = 0; i < num_voos; i++) {
        voo_t *voo = voos[i];
        if (voo_obter_status_codigo(voo) == 2)
            continue;
        destinos[voo_obter_destination_idx(voo)]++;
    }
}

static gboolean gestor_reservas_adicionar_validado(reservas_ctx_t *ctx, char **colunas,
                                                   const char *flight_ids[2], size_t num_voos,
                                                   uint32_t doc_key, double preco)
{
    gestor_reservas_t *gestor = ctx->gestor;
    gestor_voos_t *gestor_voos = ctx->gestor_voos;
    gestor_passageiros_t *gestor_passageiros = ctx->gestor_passageiros;

    uint64_t flight_keys[2] = {0, 0};
    voo_t *voos[2] = {NULL, NULL};

    if (!parser_sem_erros_ativo() && parser_dataset_grande_ativo()) {
        uint32_t reserva_key = 0;
        if (!reserva_id_key(colunas[0], &reserva_key))
            return FALSE;
        if (!reservas_ids_marcar(gestor, reserva_key))
            return FALSE;
    }

    if (parser_sem_erros_ativo()) {
        for (size_t i = 0; i < num_voos; i++)
            flight_keys[i] = flight_id_key_fast(flight_ids[i]);
    } else {
        for (size_t i = 0; i < num_voos; i++) {
            if (!utils_flight_id_key(flight_ids[i], &flight_keys[i]))
                return FALSE;
        }
    }

    passageiro_t *passageiro =
        gestor_passageiros_obter_por_documento_key(gestor_passageiros, doc_key);
    if (!passageiro)
        return FALSE;

    for (size_t i = 0; i < num_voos; i++) {
        voos[i] = gestor_voos_obter_por_key(gestor_voos, flight_keys[i]);
        if (!voos[i])
            return FALSE;
    }

    if (!parser_sem_erros_ativo() && !reserva_validar_logica(voos, num_voos, passageiro))
        return FALSE;

    gestor->total_reservas++;
    acumular_passageiros_voos_ptr(voos, num_voos);
    acumular_gastos_semana(gestor, doc_key, preco, voos, num_voos);
    acumular_destinos_nacionalidade(gestor, passageiro, voos, num_voos);
    return TRUE;
}

static void acumular_passageiros_voos_ptr(voo_t *const *voos, size_t num_voos)
{
    if (!voos || num_voos == 0)
        return;

    for (size_t i = 0; i < num_voos; i++) {
        voo_t *voo = voos[i];
        if (voo_obter_status_codigo(voo) == 2)
            continue;
        voo_incrementar_passageiros(voo, 1);
    }
}


static gboolean processar_reserva_colunas(void *contexto, char **colunas)
{
    reservas_ctx_t *ctx = contexto;
    if (!ctx || !ctx->gestor || !ctx->gestor_voos || !ctx->gestor_passageiros)
        return FALSE;

    const char *flight_ids[2] = {0};
    size_t num_voos = 0;
    const char *document_number = NULL;
    double preco = 0.0;
    uint32_t doc_key = 0;

    if (!reserva_validar_sintatica(colunas, flight_ids, &num_voos, &document_number, &doc_key,
                                   &preco))
        return FALSE;

    return gestor_reservas_adicionar_validado(ctx, colunas, flight_ids, num_voos, doc_key, preco);
}

/**
 * @brief Carrega reservas de um CSV e aplica validação.
 */
void gestor_reservas_carregar_com_validacao(gestor_reservas_t *gestor, const char *ficheiro_csv,
                                            gestor_voos_t *gestor_voos,
                                            gestor_passageiros_t *gestor_passageiros)
{
    if (!gestor || !ficheiro_csv || !gestor_voos || !gestor_passageiros)
        return;

    reservas_ctx_t ctx = {
        .gestor = gestor, .gestor_voos = gestor_voos, .gestor_passageiros = gestor_passageiros};
    parser_reservas_carregar(&ctx, ficheiro_csv, processar_reserva_colunas);
}

/**
 * @brief Compara dois gastos para ordenação decrescente.
 */
static gint cmp_gastos(gconstpointer a, gconstpointer b)
{
    const gasto_t *ga = a;
    const gasto_t *gb = b;
    if (ga->total > gb->total)
        return -1;
    if (ga->total < gb->total)
        return 1;
    if (ga->doc_key < gb->doc_key)
        return -1;
    if (ga->doc_key > gb->doc_key)
        return 1;
    return 0;
}

/**
 * @brief Insere um gasto na posição correta de um array ordenado.
 */
static void top10_inserir_ordenado(GArray *top, const gasto_t *novo)
{
    guint pos = 0;
    while (pos < top->len) {
        gasto_t *cur = &g_array_index(top, gasto_t, pos);
        if (cmp_gastos(novo, cur) < 0)
            break;
        pos++;
    }

    guint len = top->len;
    g_array_set_size(top, len + 1);
    for (guint i = len; i > pos; i--)
        g_array_index(top, gasto_t, i) = g_array_index(top, gasto_t, i - 1);
    g_array_index(top, gasto_t, pos) = *novo;
}

typedef struct {
    GArray *top;
} top10_ctx_t;

static void top10_visit(uint32_t doc_key, double total, void *user_data)
{
    top10_ctx_t *ctx = user_data;
    gasto_t g = {.doc_key = doc_key, .total = total};

    if (ctx->top->len < 10) {
        top10_inserir_ordenado(ctx->top, &g);
        return;
    }

    gasto_t *pior = &g_array_index(ctx->top, gasto_t, ctx->top->len - 1);
    if (cmp_gastos(&g, pior) < 0) {
        top10_inserir_ordenado(ctx->top, &g);
        g_array_set_size(ctx->top, 10);
    }
}

/**
 * @brief Finaliza o gestor, calculando top 10 por semana e melhor destino por nacionalidade.
 */
void gestor_reservas_finalizar(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;

    // Processa top 10 por semana
    if (gestor->gastos_por_semana) {
        GHashTableIter iter_sem;
        gpointer skey, sval;
        g_hash_table_iter_init(&iter_sem, gestor->gastos_por_semana);

        while (g_hash_table_iter_next(&iter_sem, &skey, &sval)) {
            int semana = GPOINTER_TO_INT(skey);
            doc_total_table_t *gastos = (doc_total_table_t *)sval;
            GArray *top = g_array_sized_new(FALSE, FALSE, sizeof(gasto_t), 10);

            top10_ctx_t ctx = {.top = top};
            doc_total_table_foreach(gastos, top10_visit, &ctx);

            GPtrArray *top10 = g_ptr_array_new();
            for (guint i = 0; i < top->len; i++) {
                gasto_t *g = &g_array_index(top, gasto_t, i);
                g_ptr_array_add(top10, doc_key_para_ptr(g->doc_key));
            }

            g_hash_table_insert(gestor->top10_por_semana, GINT_TO_POINTER(semana), top10);
            g_array_free(top, TRUE);
        }

        g_hash_table_destroy(gestor->gastos_por_semana);
        gestor->gastos_por_semana = NULL;
    }

    // Processa melhor destino por nacionalidade
    if (gestor->destinos_por_nacionalidade) {
        GHashTableIter iter_nac;
        gpointer nk, nv;
        g_hash_table_iter_init(&iter_nac, gestor->destinos_por_nacionalidade);

        while (g_hash_table_iter_next(&iter_nac, &nk, &nv)) {
            const char *nac = nk;
            uint32_t *destinos = nv;

            int melhor_idx = -1;
            guint melhor_count = 0;

            for (int i = 0; i < NUM_AEROPORTOS; i++) {
                guint count = destinos[i];
                if (count == 0)
                    continue;
                if (count > melhor_count ||
                    (count == melhor_count && (melhor_idx < 0 || i < melhor_idx))) {
                    melhor_idx = i;
                    melhor_count = count;
                }
            }

            if (melhor_idx >= 0) {
                melhor_destino_t *md = g_new0(melhor_destino_t, 1);
                utils_aeroporto_codigo(melhor_idx, md->destino);
                md->count = melhor_count;
                g_hash_table_insert(gestor->melhor_dest_por_nacionalidade, (gpointer)nac, md);
            }
        }

        g_hash_table_destroy(gestor->destinos_por_nacionalidade);
        gestor->destinos_por_nacionalidade = NULL;
    }
}

/**
 * @brief Retorna o top10 de uma semana.
 */
const GPtrArray *gestor_reservas_obter_top10_semana(gestor_reservas_t *gestor, int semana)
{
    if (!gestor || !gestor->top10_por_semana)
        return NULL;
    return g_hash_table_lookup(gestor->top10_por_semana, GINT_TO_POINTER(semana));
}

/**
 * @brief Obtém o melhor destino para uma nacionalidade.
 */
gboolean gestor_reservas_obter_melhor_destino_nacionalidade(gestor_reservas_t *gestor,
                                                            const char *nac, const char **destino,
                                                            guint *count)
{
    if (!gestor || !nac || !destino || !count)
        return FALSE;

    melhor_destino_t *md = g_hash_table_lookup(gestor->melhor_dest_por_nacionalidade, nac);
    if (!md || !md->destino[0])
        return FALSE;

    *destino = md->destino;
    *count = md->count;
    return TRUE;
}

/**
 * @brief Executa uma função callback para cada top10 por semana.
 */
void gestor_reservas_para_cada_top10(gestor_reservas_t *gestor,
                                     void (*callback)(int semana, const GPtrArray *top10,
                                                      void *user_data),
                                     void *user_data)
{
    if (!gestor || !callback || !gestor->top10_por_semana)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->top10_por_semana);

    while (g_hash_table_iter_next(&iter, &key, &value))
        callback(GPOINTER_TO_INT(key), value, user_data);
}

void gestor_reservas_coletar_docs_top10(gestor_reservas_t *gestor, GHashTable *doc_keys)
{
    if (!gestor || !doc_keys || !gestor->top10_por_semana)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->top10_por_semana);

    while (g_hash_table_iter_next(&iter, &key, &value)) {
        const GPtrArray *top10 = value;
        for (guint i = 0; i < top10->len; i++) {
            gpointer doc = g_ptr_array_index((GPtrArray *)top10, i);
            g_hash_table_add(doc_keys, doc);
        }
    }
}
