#include "../../include/queries/querie4.h"
#include "../../include/gestores/gestor_reservas.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/gestores/gestor_passageiros.h"
#include "../../include/entidades/reservas.h"
#include "../../include/entidades/voos.h"
#include "../../include/entidades/passageiros.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>

static inline int usa_formato_alternativo(const char *cmd)
{
    if (!cmd)
        return 0;
    while (*cmd && isspace(*cmd))
        cmd++;
    while (*cmd && isdigit(*cmd))
        cmd++;
    while (*cmd && isspace(*cmd))
        cmd++;
    return (*cmd == 'S' || *cmd == 's');
}

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
    const Gasto *ga = (const Gasto *)a;
    const Gasto *gb = (const Gasto *)b;
    if (ga->total > gb->total)
        return -1;
    if (ga->total < gb->total)
        return 1;
    return strcmp(ga->doc, gb->doc);
}

static gint cmp_resultado(gconstpointer a, gconstpointer b)
{
    const Resultado *ra = (const Resultado *)a;
    const Resultado *rb = (const Resultado *)b;
    if (ra->count != rb->count)
        return (gint)(rb->count - ra->count);

    return strcmp(ra->doc, rb->doc);
}

typedef struct
{
    const char *data_inicio;
    const char *data_fim;
    GHashTable *semanas;
    GHashTable *semanas_relevantes;
} ContextoCompleto;

void identificar_semana(int semana, reserva_t *r, void *ud)
{
    (void)r;
    ContextoCompleto *c = (ContextoCompleto *)ud;
    g_hash_table_add(c->semanas_relevantes, GINT_TO_POINTER(semana));
}

void acumular_gastos(int semana, reserva_t *r, void *ud)
{
    ContextoCompleto *c = (ContextoCompleto *)ud;

    if (!g_hash_table_contains(c->semanas_relevantes, GINT_TO_POINTER(semana)))
        return;

    GHashTable *gastos = g_hash_table_lookup(c->semanas, GINT_TO_POINTER(semana));
    if (!gastos)
    {
        gastos = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
        g_hash_table_insert(c->semanas, GINT_TO_POINTER(semana), gastos);
    }

    const char *doc = reserva_obter_document_number(r);
    double preco = reserva_obter_preco(r);

    double *total = g_hash_table_lookup(gastos, doc);
    if (total)
        *total += preco;
    else
    {
        double *novo = g_new(double, 1);
        *novo = preco;
        g_hash_table_insert(gastos, (gpointer)doc, novo);
    }
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

    ContextoCompleto ctx_full = {
        .data_inicio = data_inicio,
        .data_fim = data_fim,
        .semanas = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                                         (GDestroyNotify)g_hash_table_destroy),
        .semanas_relevantes = g_hash_table_new(g_direct_hash, g_direct_equal)};

    if (!data_inicio || !data_fim)
    {
        gestor_reservas_para_cada_com_semana(gestor_reservas, gestor_voos, identificar_semana, &ctx_full);
    }
    else
    {
        gestor_reservas_para_cada_semana(gestor_reservas, gestor_voos, data_inicio, data_fim,
                                         identificar_semana, &ctx_full);
    }

    if (g_hash_table_size(ctx_full.semanas_relevantes) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(ctx_full.semanas_relevantes);
        g_hash_table_destroy(ctx_full.semanas);
        return;
    }

    gestor_reservas_para_cada_com_semana(gestor_reservas, gestor_voos, acumular_gastos, &ctx_full);

    GHashTable *contador = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

    GHashTableIter iter_sem;
    gpointer skey, sval;
    g_hash_table_iter_init(&iter_sem, ctx_full.semanas);

    while (g_hash_table_iter_next(&iter_sem, &skey, &sval))
    {
        GHashTable *gastos = (GHashTable *)sval;
        guint num = g_hash_table_size(gastos);
        GArray *lista = g_array_sized_new(FALSE, FALSE, sizeof(Gasto), num);

        GHashTableIter iter_g;
        gpointer k, v;
        g_hash_table_iter_init(&iter_g, gastos);

        while (g_hash_table_iter_next(&iter_g, &k, &v))
        {
            Gasto g = {.doc = (char *)k, .total = *(double *)v};
            g_array_append_val(lista, g);
        }

        g_array_sort(lista, cmp_gastos);

        guint limite = (lista->len < 10) ? lista->len : 10;
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
                g_hash_table_insert(contador, g->doc, novo);
            }
        }

        g_array_free(lista, TRUE);
    }

    g_hash_table_destroy(ctx_full.semanas);
    g_hash_table_destroy(ctx_full.semanas_relevantes);

    if (g_hash_table_size(contador) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(contador);
        return;
    }

    guint num_res = g_hash_table_size(contador);
    GArray *res = g_array_sized_new(FALSE, FALSE, sizeof(Resultado), num_res);

    GHashTableIter iter;
    gpointer k, v;
    g_hash_table_iter_init(&iter, contador);

    while (g_hash_table_iter_next(&iter, &k, &v))
    {
        Resultado r = {.doc = (char *)k, .count = *(guint *)v};
        g_array_append_val(res, r);
    }

    g_array_sort(res, cmp_resultado);

    Resultado *best = &g_array_index(res, Resultado, 0);
    passageiro_t *p = gestor_passageiros_obter_por_documento(gestor_passageiros, best->doc);

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

    g_array_free(res, TRUE);
    g_hash_table_destroy(contador);
}
