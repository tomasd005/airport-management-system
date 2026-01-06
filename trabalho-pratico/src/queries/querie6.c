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

static inline int usa_formato_alternativo(const char *cmd)
{
    while (*cmd && isspace(*cmd))
        cmd++;
    while (*cmd && isdigit(*cmd))
        cmd++;
    return (*cmd == 'S');
}

void query6(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *nacionalidade,
            const char *comando_completo,
            FILE *output)
{
    if (!gestor_reservas || !gestor_voos || !gestor_passageiros || !output || !nacionalidade)
    {
        fprintf(output, "\n");
        return;
    }

    const char *sep = usa_formato_alternativo(comando_completo) ? "=" : ";";

    GPtrArray *passageiros_nac = gestor_passageiros_obter_por_nacionalidade(gestor_passageiros, nacionalidade);
    if (!passageiros_nac || passageiros_nac->len == 0)
    {
        fprintf(output, "\n");
        return;
    }

    GHashTable *destinos = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    for (guint i = 0; i < passageiros_nac->len; i++)
    {
        passageiro_t *p = g_ptr_array_index(passageiros_nac, i);
        const char *doc = passageiro_obter_document_number(p);
        if (!doc)
            continue;

        GPtrArray *reservas_pass = gestor_reservas_obter_por_passageiro(gestor_reservas, doc);
        if (!reservas_pass)
            continue;

        for (guint j = 0; j < reservas_pass->len; j++)
        {
            reserva_t *r = g_ptr_array_index(reservas_pass, j);
            size_t num_voos = reserva_obter_num_voos(r);
            const char **flight_ids = reserva_obter_flight_ids(r);

            for (size_t k = 0; k < num_voos; k++)
            {
                voo_t *v = gestor_voos_obter_por_id(gestor_voos, flight_ids[k]);
                if (!v || strcmp(voo_obter_status(v), "Cancelled") == 0)
                    continue;

                const char *dest = voo_obter_destination(v);
                if (!dest)
                    continue;

                guint *count = g_hash_table_lookup(destinos, dest);
                if (count)
                    (*count)++;
                else
                {
                    guint *novo = g_new(guint, 1);
                    *novo = 1;
                    g_hash_table_insert(destinos, g_strdup(dest), novo);
                }
            }
        }

        g_ptr_array_free(reservas_pass, TRUE);
    }

    if (g_hash_table_size(destinos) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(destinos);
        return;
    }

    const char *melhor_dest = NULL;
    guint melhor_count = 0;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, destinos);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        const char *dest = key;
        guint count = *(guint *)value;

        if (count > melhor_count || (count == melhor_count && (!melhor_dest || strcmp(dest, melhor_dest) < 0)))
        {
            melhor_count = count;
            melhor_dest = dest;
        }
    }

    if (melhor_dest)
        fprintf(output, "%s%s%u\n", melhor_dest, sep, melhor_count);
    else
        fprintf(output, "\n");

    g_hash_table_destroy(destinos);
}