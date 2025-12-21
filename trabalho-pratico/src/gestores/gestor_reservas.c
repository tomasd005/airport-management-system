#include "gestores/gestor_reservas.h"
#include "parsers/parser.h"
#include "validacoes/validacao_reservas.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct gestor_reservas
{
    GArray *reservas;
};

gestor_reservas_t *gestor_reservas_criar(void)
{
    gestor_reservas_t *g = malloc(sizeof(gestor_reservas_t));
    g->reservas = g_array_new(FALSE, FALSE, sizeof(reserva_t *));
    return g;
}

void gestor_reservas_destruir(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;
    for (guint i = 0; i < gestor->reservas->len; i++)
        reserva_destruir(g_array_index(gestor->reservas, reserva_t *, i));
    g_array_free(gestor->reservas, TRUE);
    free(gestor);
}

void gestor_reservas_adicionar(gestor_reservas_t *gestor, reserva_t *r)
{
    if (!gestor || !r)
        return;
    g_array_append_val(gestor->reservas, r);
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