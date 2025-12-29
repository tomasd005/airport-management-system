#include "../../include/queries/querie6.h"
#include "../../include/gestores/gestor_reservas.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/gestores/gestor_passageiros.h"
#include "../../include/entidades/reservas.h"
#include "../../include/entidades/voos.h"
#include "../../include/entidades/passageiros.h"
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

/* Contexto para processamento otimizado de reservas */
typedef struct
{
    gestor_voos_t *gestor_voos;
    GHashTable *passageiros_alvo; // Set de document_numbers da nacionalidade
    GHashTable *destinos;         // destino -> contador
} ContextoQ6Hibrida;

/* Processa reservas, mas filtra rapidamente usando o set */
static void processar_reserva_filtrada(const reserva_t *r, void *user_data)
{
    ContextoQ6Hibrida *ctx = user_data;

    const char *doc = reserva_obter_document_number(r);
    if (!doc)
        return;

    // VERIFICAÇÃO O(1) em vez de busca O(n)!
    if (!g_hash_table_contains(ctx->passageiros_alvo, doc))
        return; // Passageiro não é da nacionalidade desejada

    // Agora processa os voos desta reserva
    size_t num_voos = reserva_obter_num_voos(r);
    const char **flight_ids = reserva_obter_flight_ids(r);

    for (size_t j = 0; j < num_voos; j++)
    {
        voo_t *v = gestor_voos_obter_por_id(ctx->gestor_voos, flight_ids[j]);
        if (!v)
            continue;

        const char *status = voo_obter_status(v);
        if (!status || strcmp(status, "Cancelled") == 0)
            continue;

        const char *dest = voo_obter_destination(v);
        if (!dest)
            continue;

        guint *count = g_hash_table_lookup(ctx->destinos, dest);
        if (count)
            (*count)++;
        else
        {
            guint *novo = g_new(guint, 1);
            *novo = 1;
            g_hash_table_insert(ctx->destinos, g_strdup(dest), novo);
        }
    }
}

void query6(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *nacionalidade,
            const char *comando_completo,
            FILE *output)
{
    if (!gestor_reservas || !gestor_voos || !gestor_passageiros ||
        !output || !nacionalidade)
    {
        fprintf(output, "\n");
        return;
    }

    const char *sep = usa_formato_alternativo(comando_completo) ? "=" : ";";

    // FASE 1: Obter passageiros da nacionalidade (O(1) com novo índice)
    GPtrArray *passageiros_nac = gestor_passageiros_obter_por_nacionalidade(
        gestor_passageiros, nacionalidade);

    if (!passageiros_nac || passageiros_nac->len == 0)
    {
        fprintf(output, "\n");
        return;
    }

    // FASE 2: Criar set de document_numbers (O(P) onde P = passageiros da nacionalidade)
    GHashTable *passageiros_alvo = g_hash_table_new(g_str_hash, g_str_equal);

    for (guint i = 0; i < passageiros_nac->len; i++)
    {
        passageiro_t *p = g_ptr_array_index(passageiros_nac, i);
        const char *doc = passageiro_obter_document_number(p);
        if (doc)
            g_hash_table_add(passageiros_alvo, (gpointer)doc);
    }

    // FASE 3: Processar reservas com filtro O(1) (O(R) onde R = reservas)
    ContextoQ6Hibrida ctx = {
        .gestor_voos = gestor_voos,
        .passageiros_alvo = passageiros_alvo,
        .destinos = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free)};

    gestor_reservas_para_cada(gestor_reservas, processar_reserva_filtrada, &ctx);

    // FASE 4: Encontrar destino mais comum
    if (g_hash_table_size(ctx.destinos) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(ctx.destinos);
        g_hash_table_destroy(passageiros_alvo);
        return;
    }

    const char *melhor_dest = NULL;
    guint melhor_count = 0;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, ctx.destinos);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char *dest = key;
        guint count = *(guint *)value;

        if (count > melhor_count ||
            (count == melhor_count && (melhor_dest == NULL || strcmp(dest, melhor_dest) < 0)))
        {
            melhor_count = count;
            melhor_dest = dest;
        }
    }

    if (melhor_dest)
        fprintf(output, "%s%s%u\n", melhor_dest, sep, melhor_count);
    else
        fprintf(output, "\n");

    g_hash_table_destroy(ctx.destinos);
    g_hash_table_destroy(passageiros_alvo);
}