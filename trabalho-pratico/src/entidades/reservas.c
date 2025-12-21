#include "reservas.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdbool.h>

struct reserva
{
    char *reservation_id;
    char **flight_ids;
    size_t num_voos;
    char *document_number;
    char *seat;
    double preco;
    bool extra_bagagem;
    bool embarque_prioritario;
    char *qr_code;
};

reserva_t *reserva_criar(const char *reservation_id, const char **flight_ids,
                         size_t num_flights, const char *document_number,
                         const char *seat, double preco, bool extra_bagagem,
                         bool embarque_prioritario, const char *qr_code)
{
    if (!reservation_id || !flight_ids || num_flights == 0 ||
        !document_number || !seat || !qr_code)
        return NULL;

    reserva_t *r = malloc(sizeof(reserva_t));
    if (!r)
        return NULL;

    r->flight_ids = NULL;
    r->num_voos = 0;
    r->reservation_id = NULL;
    r->document_number = NULL;
    r->seat = NULL;
    r->qr_code = NULL;

    r->reservation_id = g_strdup(reservation_id);
    r->document_number = g_strdup(document_number);
    r->seat = g_strdup(seat);
    r->qr_code = g_strdup(qr_code);
    r->preco = preco;
    r->extra_bagagem = extra_bagagem;
    r->embarque_prioritario = embarque_prioritario;
    r->num_voos = num_flights;

    r->flight_ids = malloc(sizeof(char *) * num_flights);
    if (!r->flight_ids)
    {
        reserva_destruir(r);
        return NULL;
    }

    for (size_t i = 0; i < num_flights; i++)
    {
        r->flight_ids[i] = g_strdup(flight_ids[i]);
        if (!r->flight_ids[i])
        {
            reserva_destruir(r);
            return NULL;
        }
    }

    if (!r->reservation_id || !r->document_number || !r->seat || !r->qr_code)
    {
        reserva_destruir(r);
        return NULL;
    }

    return r;
}

void reserva_destruir(reserva_t *r)
{
    if (!r)
        return;

    if (r->reservation_id)
        g_free(r->reservation_id);
    if (r->document_number)
        g_free(r->document_number);
    if (r->seat)
        g_free(r->seat);
    if (r->qr_code)
        g_free(r->qr_code);

    if (r->flight_ids)
    {
        for (size_t i = 0; i < r->num_voos; i++)
        {
            if (r->flight_ids[i])
                g_free(r->flight_ids[i]);
        }
        free(r->flight_ids);
    }

    free(r);
}

const char *reserva_obter_id(const reserva_t *r)
{
    return r ? r->reservation_id : NULL;
}

size_t reserva_obter_num_voos(const reserva_t *r)
{
    return r ? r->num_voos : 0;
}

const char **reserva_obter_flight_ids(const reserva_t *r)
{
    return r ? (const char **)r->flight_ids : NULL;
}

const char *reserva_obter_document_number(const reserva_t *r)
{
    return r ? r->document_number : NULL;
}

const char *reserva_obter_seat(const reserva_t *r)
{
    return r ? r->seat : NULL;
}

double reserva_obter_preco(const reserva_t *r)
{
    return r ? r->preco : 0.0;
}

bool reserva_obter_extra_bagagem(const reserva_t *r)
{
    return r ? r->extra_bagagem : false;
}

bool reserva_obter_embarque_prioritario(const reserva_t *r)
{
    return r ? r->embarque_prioritario : false;
}

const char *reserva_obter_qr_code(const reserva_t *r)
{
    return r ? r->qr_code : NULL;
}