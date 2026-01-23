#include "gestores/gestor_reservas.h"
#include "gestores/gestor_passageiros.h"
#include "gestores/gestor_voos.h"
#include "parsers/parser_reservas.h"
#include "validacoes/validacao_reservas.h"
#include "entidades/passageiros.h"
#include "entidades/voos.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

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

static inline gpointer doc_key_para_ptr(uint32_t key)
{
    return GUINT_TO_POINTER((guint)(key + 1));
}

static inline uint32_t doc_key_de_ptr(gpointer ptr)
{
    return (uint32_t)(GPOINTER_TO_UINT(ptr) - 1);
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
};

/**
 * @brief Cria um mapa de gastos (documento -> total gasto).
 * @return Novo GHashTable.
 */
static GHashTable *criar_mapa_gastos(void)
{
    return g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
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
                                                 (GDestroyNotify)g_hash_table_destroy);
    g->top10_por_semana = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                                                (GDestroyNotify)g_ptr_array_unref);
    g->destinos_por_nacionalidade = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
    g->melhor_dest_por_nacionalidade = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
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

    GHashTable *gastos = g_hash_table_lookup(gestor->gastos_por_semana, GINT_TO_POINTER(semana));
    if (!gastos) {
        gastos = criar_mapa_gastos();
        g_hash_table_insert(gestor->gastos_por_semana, GINT_TO_POINTER(semana), gastos);
    }

    gpointer k = doc_key_para_ptr(doc_key);
    double *total = g_hash_table_lookup(gastos, k);
    if (total)
        *total += preco;
    else {
        double *novo = g_new(double, 1);
        *novo = preco;
        g_hash_table_insert(gastos, k, novo);
    }
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
        if (!voo)
            continue;

        if (voo_obter_status_codigo(voo) == 2)
            continue;

        int dest_idx = voo_obter_destination_idx(voo);
        if (dest_idx < 0 || dest_idx >= NUM_AEROPORTOS)
            continue;
        destinos[dest_idx]++;
    }
}

static void acumular_passageiros_voos_ptr(voo_t *const *voos, size_t num_voos)
{
    if (!voos || num_voos == 0)
        return;

    for (size_t i = 0; i < num_voos; i++) {
        voo_t *voo = voos[i];
        if (!voo)
            continue;
        voo_incrementar_passageiros(voo, 1);
    }
}

/**
 * @brief Verifica a validade lógica de uma reserva antes do carregamento.
 */
static gboolean reserva_valida_logica_view(voo_t *const *voos, size_t num_voos,
                                           passageiro_t *passageiro)
{
    if (!voos || num_voos == 0 || !passageiro)
        return FALSE;

    if (!voos[0])
        return FALSE;

    if (num_voos == 2 && voos[1]) {
        int dest1 = voo_obter_destination_idx(voos[0]);
        int orig2 = voo_obter_origin_idx(voos[1]);
        if (dest1 < 0 || orig2 < 0 || dest1 != orig2)
            return FALSE;
    }

    return TRUE;
}

static gboolean processar_reserva_colunas(void *contexto, char **colunas)
{
    reservas_ctx_t *ctx = contexto;
    if (!ctx || !ctx->gestor || !ctx->gestor_voos || !ctx->gestor_passageiros)
        return FALSE;

    gestor_reservas_t *gestor = ctx->gestor;
    gestor_voos_t *gestor_voos = ctx->gestor_voos;
    gestor_passageiros_t *gestor_passageiros = ctx->gestor_passageiros;

    const char *flight_ids[2] = {0};
    size_t num_voos = 0;
    const char *document_number = NULL;
    double preco = 0.0;
    uint32_t doc_key = 0;
    uint64_t flight_keys[2] = {0, 0};
    voo_t *voos[2] = {NULL, NULL};

    if (!valida_reserva_campos(colunas, flight_ids, &num_voos, &document_number, &doc_key, &preco))
        return FALSE;

    for (size_t i = 0; i < num_voos; i++) {
        if (!utils_flight_id_key(flight_ids[i], &flight_keys[i]))
            return FALSE;
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

    if (!reserva_valida_logica_view(voos, num_voos, passageiro))
        return FALSE;

    gestor->total_reservas++;
    acumular_passageiros_voos_ptr(voos, num_voos);
    acumular_gastos_semana(gestor, doc_key, preco, voos, num_voos);
    acumular_destinos_nacionalidade(gestor, passageiro, voos, num_voos);
    return TRUE;
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
            GHashTable *gastos = (GHashTable *)sval;
            GArray *top = g_array_sized_new(FALSE, FALSE, sizeof(gasto_t), 10);

            GHashTableIter iter_g;
            gpointer k, v;
            g_hash_table_iter_init(&iter_g, gastos);

            while (g_hash_table_iter_next(&iter_g, &k, &v)) {
                gasto_t g = {.doc_key = doc_key_de_ptr(k), .total = *(double *)v};
                if (top->len < 10) {
                    top10_inserir_ordenado(top, &g);
                } else {
                    gasto_t *pior = &g_array_index(top, gasto_t, top->len - 1);
                    if (cmp_gastos(&g, pior) < 0) {
                        top10_inserir_ordenado(top, &g);
                        g_array_set_size(top, 10);
                    }
                }
            }

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
