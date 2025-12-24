#include "gestores/gestor_reservas.h"
#include "parsers/parser.h"
#include "validacoes/validacao_reservas.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct gestor_reservas
{
    GArray *reservas;          // Array de todas as reservas
    GHashTable *por_flight_id; // flight_id -> contagem de passageiros
};

gestor_reservas_t *gestor_reservas_criar(void)
{
    gestor_reservas_t *g = malloc(sizeof(gestor_reservas_t));
    g->reservas = g_array_new(FALSE, FALSE, sizeof(reserva_t *));

    // Hash table: flight_id -> número de passageiros (int)
    g->por_flight_id = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        g_free);

    return g;
}

void gestor_reservas_destruir(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;
    for (guint i = 0; i < gestor->reservas->len; i++)
        reserva_destruir(g_array_index(gestor->reservas, reserva_t *, i));
    g_array_free(gestor->reservas, TRUE);
    g_hash_table_destroy(gestor->por_flight_id);
    free(gestor);
}

void gestor_reservas_adicionar(gestor_reservas_t *gestor, reserva_t *r)
{
    if (!gestor || !r)
        return;

    g_array_append_val(gestor->reservas, r);

    // Atualiza índice por flight_id
    const char **flight_ids = reserva_obter_flight_ids(r);
    size_t num_voos = reserva_obter_num_voos(r);

    for (size_t i = 0; i < num_voos; i++)
    {
        const char *fid = flight_ids[i];

        int *count = g_hash_table_lookup(gestor->por_flight_id, fid);
        if (count)
        {
            (*count)++;
        }
        else
        {
            int *new_count = g_new(int, 1);
            *new_count = 1;
            g_hash_table_insert(gestor->por_flight_id, g_strdup(fid), new_count);
        }
    }
}

reserva_t *gestor_reservas_obter_por_id(gestor_reservas_t *gestor, const char *reservation_id)
{
    if (!gestor || !reservation_id)
        return NULL;

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_array_index(gestor->reservas, reserva_t *, i);
        if (strcmp(reserva_obter_id(r), reservation_id) == 0)
            return r;
    }
    return NULL;
}

int gestor_reservas_contar_passageiros_voo(gestor_reservas_t *gestor, const char *flight_id)
{
    if (!gestor || !flight_id)
        return 0;

    int *count = g_hash_table_lookup(gestor->por_flight_id, flight_id);
    return count ? *count : 0;
}

unsigned int gestor_reservas_numero(gestor_reservas_t *gestor)
{
    return gestor ? gestor->reservas->len : 0;
}

// Callback interno para o parser
static gboolean adiciona_reserva_callback(void *contexto, void *objeto)
{
    gestor_reservas_t *gestor = (gestor_reservas_t *)contexto;
    reserva_t *reserva = (reserva_t *)objeto;

    if (!gestor || !reserva)
        return FALSE;

    gestor_reservas_adicionar(gestor, reserva);
    return TRUE;
}

void gestor_reservas_carregar(gestor_reservas_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    parser_carrega(
        gestor,
        ficheiro_csv,
        adiciona_reserva_callback,
        (LinhaParaObjeto)valida_reserva_from_csv,
        (DestroiObjeto)reserva_destruir);
}