#include "../../include/queries/querie4.h"
#include "../../include/gestores/gestor_reservas.h"
#include "../../include/gestores/gestor_passageiros.h"
#include "../../include/entidades/passageiros.h"
#include "../../include/utils.h"
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
    GHashTable *contador;
} ContadorTop10Ctx;

static void contar_semana_top10(int semana, const GPtrArray *top10, void *user_data)
{
    (void)semana;
    ContadorTop10Ctx *ctx = user_data;
    if (!ctx || !top10)
        return;

    for (guint i = 0; i < top10->len; i++)
    {
        const char *doc = g_ptr_array_index((GPtrArray *)top10, i);
        guint *c = g_hash_table_lookup(ctx->contador, doc);
        if (c)
            (*c)++;
        else
        {
            guint *novo = g_new(guint, 1);
            *novo = 1;
            g_hash_table_insert(ctx->contador, (gpointer)doc, novo);
        }
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
    if (!gestor_reservas || !gestor_passageiros || !output)
    {
        fprintf(output, "\n");
        return;
    }

    (void)gestor_voos;

    const char *sep = usa_formato_alternativo(comando_completo) ? "=" : ";";

    GHashTable *contador = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

    if (data_inicio && data_fim)
    {
        int dia_inicio = utils_parse_date_to_day(data_inicio);
        int dia_fim = utils_parse_date_to_day(data_fim);
        if (dia_inicio < 0 || dia_fim < 0 || dia_inicio > dia_fim)
        {
            fprintf(output, "\n");
            g_hash_table_destroy(contador);
            return;
        }

        int semana_inicio = utils_week_from_day(dia_inicio);
        int semana_fim = utils_week_from_day(dia_fim);
        for (int semana = semana_inicio; semana <= semana_fim; semana++)
        {
            const GPtrArray *top10 = gestor_reservas_obter_top10_semana(gestor_reservas, semana);
            if (!top10)
                continue;
            for (guint i = 0; i < top10->len; i++)
            {
                const char *doc = g_ptr_array_index((GPtrArray *)top10, i);
                guint *c = g_hash_table_lookup(contador, doc);
                if (c)
                    (*c)++;
                else
                {
                    guint *novo = g_new(guint, 1);
                    *novo = 1;
                    g_hash_table_insert(contador, (gpointer)doc, novo);
                }
            }
        }
    }
    else
    {
        ContadorTop10Ctx ctx = {.contador = contador};
        gestor_reservas_para_cada_top10(gestor_reservas, contar_semana_top10, &ctx);
    }

    if (g_hash_table_size(contador) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(contador);
        return;
    }

    const char *melhor_doc = NULL;
    guint melhor_count = 0;

    GHashTableIter iter;
    gpointer k, v;
    g_hash_table_iter_init(&iter, contador);
    while (g_hash_table_iter_next(&iter, &k, &v))
    {
        const char *doc = k;
        guint count = *(guint *)v;
        if (count > melhor_count || (count == melhor_count && (!melhor_doc || strcmp(doc, melhor_doc) < 0)))
        {
            melhor_doc = doc;
            melhor_count = count;
        }
    }

    passageiro_t *p = melhor_doc ? gestor_passageiros_obter_por_documento(gestor_passageiros, melhor_doc) : NULL;
    if (p)
    {
        fprintf(output, "%s%s%s%s%s%s%s%s%s%s%u\n",
                passageiro_obter_document_number(p), sep,
                passageiro_obter_primeiro_nome(p), sep,
                passageiro_obter_ultimo_nome(p), sep,
                passageiro_obter_dob(p), sep,
                passageiro_obter_nacionalidade(p), sep,
                melhor_count);
    }
    else
        fprintf(output, "\n");

    g_hash_table_destroy(contador);
}
