#include "gestores/gestor_reservas.h"
#include "parsers/parser.h"
#include "validacoes/validacao_reservas.h"
#include "gestores/gestor_voos.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>


struct gestor_reservas
{
    GArray *reservas;          // Array de todas as reservas
    GHashTable *por_flight_id; // Mantido por compatibilidade mas não usado
};

gestor_reservas_t *gestor_reservas_criar(void)
{
    gestor_reservas_t *g = malloc(sizeof(gestor_reservas_t));
    g->reservas = g_array_new(FALSE, FALSE, sizeof(reserva_t *));

    // Mant ido por compatibilidade
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

/**
 * Conta passageiros ÚNICOS num voo específico
 * - Itera todas as reservas
 * - Para cada reserva, verifica se contém o flight_id
 * - Usa um set de reservation_ids para garantir contagem única
 */
int gestor_reservas_contar_passageiros_voo(gestor_reservas_t *gestor, const char *flight_id)
{
    if (!gestor || !flight_id)
        return 0;

    // Set para garantir que cada reserva é contada apenas uma vez
    GHashTable *reservas_unicas = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        NULL);

    // Itera todas as reservas
    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_array_index(gestor->reservas, reserva_t *, i);

        const char **flight_ids = reserva_obter_flight_ids(r);
        size_t num_voos = reserva_obter_num_voos(r);

        // Verifica se esta reserva contém o flight_id procurado
        for (size_t j = 0; j < num_voos; j++)
        {
            if (strcmp(flight_ids[j], flight_id) == 0)
            {
                // Adiciona ao set (não duplica se já existir)
                const char *reservation_id = reserva_obter_id(r);
                if (reservation_id && !g_hash_table_contains(reservas_unicas, reservation_id))
                {
                    g_hash_table_insert(reservas_unicas, g_strdup(reservation_id), GINT_TO_POINTER(1));
                }
                break; // Não precisa verificar outros voos desta reserva
            }
        }
    }

    int count = g_hash_table_size(reservas_unicas);
    g_hash_table_destroy(reservas_unicas);

    return count;
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

static int calcular_semana(const char *data)
{
    int y, m, d;
    sscanf(data, "%d-%d-%d", &y, &m, &d);

    struct tm t = {0};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    mktime(&t);

    return (y * 1000) + (t.tm_yday - t.tm_wday); // domingo
}

void gestor_reservas_para_cada_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    const char *data_inicio,
    const char *data_fim,
    void (*callback)(int, const reserva_t *, void *),
    void *user_data)
{
    if (!gestor || !gestor_voos || !callback)
        return;

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_array_index(gestor->reservas, reserva_t *, i);

        const char **flight_ids = reserva_obter_flight_ids(r);
        const char *dep = gestor_voos_obter_departure(
            gestor_voos, flight_ids[0]);

        if (!dep || strlen(dep) < 10)
            continue;

        char data[11];
        strncpy(data, dep, 10);
        data[10] = '\0';

        if (data_inicio && strcmp(data, data_inicio) < 0)
            continue;
        if (data_fim && strcmp(data, data_fim) > 0)
            continue;

        int semana = calcular_semana(data);
        callback(semana, r, user_data);
    }
}


