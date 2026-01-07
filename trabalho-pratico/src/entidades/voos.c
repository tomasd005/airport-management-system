#include "voos.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct voo
{
    char *flight_id;
    const char *origin;
    const char *destination;
    char *aircraft;
    char *airline;
    int dep_day;
    int act_dep_day;
    int semana;
    int passageiros;
    int atraso_min;
    unsigned char status;
};

enum
{
    VOO_STATUS_ON_TIME = 0,
    VOO_STATUS_DELAYED = 1,
    VOO_STATUS_CANCELLED = 2
};

static GHashTable *intern_pool = NULL;

static const char *voo_intern_string(const char *s)
{
    if (!s)
        return NULL;

    if (!intern_pool)
        intern_pool = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    gpointer existente = g_hash_table_lookup(intern_pool, s);
    if (existente)
        return existente;

    char *dup = g_strdup(s);
    g_hash_table_insert(intern_pool, dup, dup);
    return dup;
}

static unsigned char status_from_str(const char *status)
{
    if (!status)
        return VOO_STATUS_ON_TIME;
    if (strcmp(status, "Cancelled") == 0)
        return VOO_STATUS_CANCELLED;
    if (strcmp(status, "Delayed") == 0)
        return VOO_STATUS_DELAYED;
    return VOO_STATUS_ON_TIME;
}

static const char *status_to_str(unsigned char status)
{
    switch (status)
    {
    case VOO_STATUS_DELAYED:
        return "Delayed";
    case VOO_STATUS_CANCELLED:
        return "Cancelled";
    default:
        return "On Time";
    }
}

voo_t *voo_criar(const char *flight_id, const char *departure,
                 const char *actual_departure, const char *arrival,
                 const char *actual_arrival, const char *gate,
                 const char *status, const char *origin,
                 const char *destination, const char *aircraft,
                 const char *airline, const char *tracking_url)
{
    if (!flight_id || !departure || !status || !origin || !destination || !aircraft)
        return NULL;

    voo_t *v = malloc(sizeof(voo_t));
    if (!v)
        return NULL;

    v->flight_id = g_strdup(flight_id);
    v->origin = voo_intern_string(origin);
    v->destination = voo_intern_string(destination);
    v->aircraft = aircraft ? g_strdup(aircraft) : NULL;
    v->airline = airline ? g_strdup(airline) : NULL;
    v->status = status_from_str(status);
    v->dep_day = utils_parse_datetime_to_day(departure);
    v->act_dep_day = utils_parse_datetime_to_day(actual_departure);
    v->semana = utils_week_from_day(v->dep_day);
    v->passageiros = 0;
    v->atraso_min = -1;
    if (actual_departure && strcmp(actual_departure, "N/A") != 0)
    {
        int dep_min = utils_parse_datetime_to_minutes(departure);
        int act_min = utils_parse_datetime_to_minutes(actual_departure);
        if (dep_min >= 0 && act_min >= 0 && act_min >= dep_min)
            v->atraso_min = act_min - dep_min;
    }

    if (!v->flight_id || !v->origin || !v->destination)
    {
        voo_destruir(v);
        return NULL;
    }

    return v;
}

void voo_destruir(voo_t *v)
{
    if (!v)
        return;

    free(v->flight_id);
    if (v->aircraft)
        free(v->aircraft);
    if (v->airline)
        free(v->airline);
    free(v);
}

const char *voo_obter_id(const voo_t *v) { return v ? v->flight_id : NULL; }
const char *voo_obter_departure(const voo_t *v) { (void)v; return NULL; }
const char *voo_obter_actual_departure(const voo_t *v) { (void)v; return NULL; }
const char *voo_obter_arrival(const voo_t *v) { return v ? "N/A" : NULL; }
const char *voo_obter_actual_arrival(const voo_t *v) { return v ? "N/A" : NULL; }
const char *voo_obter_gate(const voo_t *v) { return v ? "" : NULL; }
const char *voo_obter_status(const voo_t *v) { return v ? status_to_str(v->status) : NULL; }
const char *voo_obter_origin(const voo_t *v) { return v ? v->origin : NULL; }
const char *voo_obter_destination(const voo_t *v) { return v ? v->destination : NULL; }
const char *voo_obter_aircraft(const voo_t *v) { return v ? v->aircraft : NULL; }
const char *voo_obter_airline(const voo_t *v) { return v ? v->airline : NULL; }
const char *voo_obter_tracking_url(const voo_t *v) { return v ? "" : NULL; }

double voo_calcular_atraso_minutos(const voo_t *v)
{
    if (!v)
        return -1.0;
    return (v->atraso_min >= 0) ? (double)v->atraso_min : -1.0;
}

int voo_obter_departure_dia(const voo_t *v)
{
    return v ? v->dep_day : -1;
}

int voo_obter_actual_departure_dia(const voo_t *v)
{
    return v ? v->act_dep_day : -1;
}

int voo_obter_semana(const voo_t *v)
{
    return v ? v->semana : -1;
}

int voo_obter_status_codigo(const voo_t *v)
{
    return v ? (int)v->status : -1;
}

int voo_obter_passageiros(const voo_t *v)
{
    return v ? v->passageiros : 0;
}

void voo_incrementar_passageiros(voo_t *v, int delta)
{
    if (!v)
        return;
    v->passageiros += delta;
}

void voo_descartar_aircraft(voo_t *v)
{
    if (!v || !v->aircraft)
        return;
    free(v->aircraft);
    v->aircraft = NULL;
}

void voo_descartar_airline(voo_t *v)
{
    if (!v || !v->airline)
        return;
    free(v->airline);
    v->airline = NULL;
}

void voo_intern_pool_destruir(void)
{
    if (!intern_pool)
        return;
    g_hash_table_destroy(intern_pool);
    intern_pool = NULL;
}
