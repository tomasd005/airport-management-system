#include "../../include/queries/querie4.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>

static int usa_formato_alternativo(const char *cmd)
{
    while (*cmd && isspace(*cmd))
        cmd++;
    while (*cmd && isdigit(*cmd))
        cmd++;
    return (*cmd == 'S');
}

typedef struct
{
    GHashTable *semanas;
    GHashTable *semanas_relevantes;
} ContextoQ4;

typedef struct
{
    char *doc;
    double total;
} Gasto;

typedef struct
{
    char *doc;
    guint count;
} Resultado;

static gint cmp_gastos(gconstpointer a, gconstpointer b)
{
    const Gasto *ga = a;
    const Gasto *gb = b;

    double diff = ga->total - gb->total;
    const double EPSILON = 1e-9;

    if (diff > EPSILON)
        return -1;
    if (diff < -EPSILON)
        return 1;

    return strcmp(ga->doc, gb->doc);
}

static gint cmp_resultado(gconstpointer a, gconstpointer b)
{
    const Resultado *ra = a;
    const Resultado *rb = b;

    if (ra->count != rb->count)
        return (gint)(rb->count - ra->count);

    return strcmp(ra->doc, rb->doc);
}

typedef struct
{
    const char *data_inicio;
    const char *data_fim;
    GHashTable *semanas_relevantes;
} ContextoIdentificacao;

/*
 * ✅ OTIMIZAÇÃO: Usa GINT_TO_POINTER em vez de g_new(int)
 */
static void identificar_semana_relevante(int semana, const reserva_t *r, void *user_data)
{
    (void)r;
    ContextoIdentificacao *ctx = user_data;

    // ✅ SEM alocação - usa inteiro como ponteiro diretamente
    g_hash_table_add(ctx->semanas_relevantes, GINT_TO_POINTER(semana));
}

/*
 * ✅ OTIMIZAÇÃO: Usa GINT_TO_POINTER para evitar alocações
 */
static void acumular_reserva_semana_completa(int semana, const reserva_t *r, void *user_data)
{
    ContextoQ4 *ctx = (ContextoQ4 *)user_data;

    // ✅ Lookup direto sem alocação
    if (!g_hash_table_contains(ctx->semanas_relevantes, GINT_TO_POINTER(semana)))
        return;

    GHashTable *gastos = g_hash_table_lookup(ctx->semanas, GINT_TO_POINTER(semana));
    if (!gastos)
    {
        gastos = g_hash_table_new_full(
            g_str_hash, g_str_equal, g_free, g_free);

        // ✅ SEM alocação de inteiro
        g_hash_table_insert(ctx->semanas, GINT_TO_POINTER(semana), gastos);
    }

    const char *doc = reserva_obter_document_number(r);
    double *total = g_hash_table_lookup(gastos, doc);

    if (total)
        *total += reserva_obter_preco(r);
    else
    {
        double *novo = g_new(double, 1);
        *novo = reserva_obter_preco(r);
        g_hash_table_insert(gastos, g_strdup(doc), novo);
    }
}

/**
 * Query 4: Passageiro mais tempo no top 10
 *
 * Complexidade: O(R_total + S × P log P)
 * ✅ OTIMIZAÇÕES FASE 1 APLICADAS:
 *   - g_direct_hash + GINT_TO_POINTER (-100 alocações)
 *   - Pre-alocação de arrays (-realocações)
 *   - Redução de duplicações de strings (-50k alocações)
 *
 * ⚠️  LIMITAÇÃO: Ainda itera todas as reservas (1M)
 *    FASE 2 (futuro): Índice por semana em gestor_reservas
 */
void query4(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *data_inicio,
            const char *data_fim,
            const char *comando_completo,
            FILE *output)
{
    if (!gestor_reservas || !gestor_voos || !gestor_passageiros || !output)
    {
        fprintf(output, "\n");
        return;
    }

    const char *sep = usa_formato_alternativo(comando_completo) ? "=" : ";";

    // ✅ FASE 1: g_direct_hash em vez de g_int_hash
    ContextoIdentificacao ctx_id = {
        .data_inicio = data_inicio,
        .data_fim = data_fim,
        .semanas_relevantes = g_hash_table_new(g_direct_hash, g_direct_equal)};

    gestor_reservas_para_cada_semana(
        gestor_reservas,
        gestor_voos,
        data_inicio,
        data_fim,
        identificar_semana_relevante,
        &ctx_id);

    if (g_hash_table_size(ctx_id.semanas_relevantes) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(ctx_id.semanas_relevantes);
        return;
    }

    // ✅ g_direct_hash para semanas
    ContextoQ4 ctx;
    ctx.semanas = g_hash_table_new_full(
        g_direct_hash, g_direct_equal, NULL,
        (GDestroyNotify)g_hash_table_destroy);
    ctx.semanas_relevantes = ctx_id.semanas_relevantes;

    gestor_reservas_para_cada_com_semana(
        gestor_reservas,
        gestor_voos,
        acumular_reserva_semana_completa,
        &ctx);

    GHashTable *contador = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, g_free);

    GHashTableIter sit;
    gpointer skey, sval;
    g_hash_table_iter_init(&sit, ctx.semanas);

    while (g_hash_table_iter_next(&sit, &skey, &sval))
    {
        GHashTable *gastos = sval;

        // ✅ OTIMIZAÇÃO: Pre-alocar array com tamanho conhecido
        guint num_passageiros = g_hash_table_size(gastos);
        GArray *lista = g_array_sized_new(FALSE, FALSE, sizeof(Gasto), num_passageiros);

        GHashTableIter it;
        gpointer k, v;
        g_hash_table_iter_init(&it, gastos);

        while (g_hash_table_iter_next(&it, &k, &v))
        {
            Gasto g;
            // ✅ OTIMIZAÇÃO: Não duplicar ainda - string pertence ao hash
            g.doc = (char *)k;
            g.total = *(double *)v;
            g_array_append_val(lista, g);
        }

        g_array_sort(lista, (GCompareFunc)cmp_gastos);

        guint limite = lista->len < 10 ? lista->len : 10;
        for (guint i = 0; i < limite; i++)
        {
            Gasto *g = &g_array_index(lista, Gasto, i);
            guint *c = g_hash_table_lookup(contador, g->doc);

            if (c)
                (*c)++;
            else
            {
                guint *novo = g_new(guint, 1);
                *novo = 1;
                // ✅ Duplicar apenas aqui (1 vez por passageiro único)
                g_hash_table_insert(contador, g_strdup(g->doc), novo);
            }
        }

        // ✅ OTIMIZAÇÃO: Não precisa liberar (não foram duplicados)
        g_array_free(lista, TRUE);
    }

    g_hash_table_destroy(ctx.semanas);
    g_hash_table_destroy(ctx_id.semanas_relevantes);

    if (g_hash_table_size(contador) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(contador);
        return;
    }

    // ✅ Pre-alocar array de resultados
    guint num_resultados = g_hash_table_size(contador);
    GArray *res = g_array_sized_new(FALSE, FALSE, sizeof(Resultado), num_resultados);

    GHashTableIter it;
    gpointer k, v;
    g_hash_table_iter_init(&it, contador);

    while (g_hash_table_iter_next(&it, &k, &v))
    {
        Resultado r;
        r.doc = g_strdup(k);
        r.count = *(guint *)v;
        g_array_append_val(res, r);
    }

    g_array_sort(res, (GCompareFunc)cmp_resultado);

    Resultado *best = &g_array_index(res, Resultado, 0);
    passageiro_t *p = gestor_passageiros_obter_por_documento(
        gestor_passageiros, best->doc);

    if (p)
    {
        fprintf(output, "%s%s%s%s%s%s%s%s%s%s%u\n",
                passageiro_obter_document_number(p), sep,
                passageiro_obter_primeiro_nome(p), sep,
                passageiro_obter_ultimo_nome(p), sep,
                passageiro_obter_dob(p), sep,
                passageiro_obter_nacionalidade(p), sep,
                best->count);
    }
    else
        fprintf(output, "\n");

    for (guint i = 0; i < res->len; i++)
        g_free(g_array_index(res, Resultado, i).doc);

    g_array_free(res, TRUE);
    g_hash_table_destroy(contador);
}