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

static void acumular_reserva(
    int semana,
    const reserva_t *r,
    void *user_data)
{
    ContextoQ4 *ctx = user_data;

    GHashTable *gastos = g_hash_table_lookup(ctx->semanas, &semana);
    if (!gastos)
    {
        int *k = g_new(int, 1);
        *k = semana;

        gastos = g_hash_table_new_full(
            g_str_hash, g_str_equal, g_free, g_free);

        g_hash_table_insert(ctx->semanas, k, gastos);
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

static gint cmp_gastos(gconstpointer a, gconstpointer b)
{
    const Gasto *ga = a;
    const Gasto *gb = b;

    if (ga->total != gb->total)
        return (ga->total < gb->total) ? 1 : -1;

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

    ContextoQ4 ctx;
    ctx.semanas = g_hash_table_new_full(
        g_int_hash, g_int_equal, g_free,
        (GDestroyNotify)g_hash_table_destroy);

    gestor_reservas_para_cada_semana(
        gestor_reservas,
        gestor_voos,
        data_inicio,
        data_fim,
        acumular_reserva,
        &ctx);

    GHashTable *contador = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, g_free);

    GHashTableIter sit;
    gpointer skey, sval;
    g_hash_table_iter_init(&sit, ctx.semanas);

    while (g_hash_table_iter_next(&sit, &skey, &sval))
    {
        GHashTable *gastos = sval;
        GArray *lista = g_array_new(FALSE, FALSE, sizeof(Gasto));

        GHashTableIter it;
        gpointer k, v;
        g_hash_table_iter_init(&it, gastos);

        while (g_hash_table_iter_next(&it, &k, &v))
        {
            Gasto g;
            g.doc = g_strdup(k);
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
                g_hash_table_insert(contador, g_strdup(g->doc), novo);
            }
        }

        for (guint i = 0; i < lista->len; i++)
            g_free(g_array_index(lista, Gasto, i).doc);

        g_array_free(lista, TRUE);
    }

    g_hash_table_destroy(ctx.semanas);

    if (g_hash_table_size(contador) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(contador);
        return;
    }

    GArray *res = g_array_new(FALSE, FALSE, sizeof(Resultado));

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