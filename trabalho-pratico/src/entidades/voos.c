#include "voos.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>

struct voo
{
    char *flight_id;
    char *departure;
    char *actual_departure;
    char *arrival;
    char *actual_arrival;
    char *gate;
    char *status;
    char *origin;
    char *destination;
    char *aircraft;
    char *airline;
    char *tracking_url;
};

voo_t *voo_criar(const char *flight_id, const char *departure,
                 const char *actual_departure, const char *arrival,
                 const char *actual_arrival, const char *gate,
                 const char *status, const char *origin,
                 const char *destination, const char *aircraft,
                 const char *airline, const char *tracking_url)
{
    if (!flight_id || !departure || !arrival || !status ||
        !origin || !destination || !aircraft)
        return NULL;

    voo_t *v = malloc(sizeof(voo_t));
    if (!v)
        return NULL;

    v->flight_id = g_strdup(flight_id);
    v->departure = g_strdup(departure);
    v->actual_departure = actual_departure ? g_strdup(actual_departure) : g_strdup("N/A");
    v->arrival = g_strdup(arrival);
    v->actual_arrival = actual_arrival ? g_strdup(actual_arrival) : g_strdup("N/A");
    v->gate = gate ? g_strdup(gate) : g_strdup("");
    v->status = g_strdup(status);
    v->origin = g_strdup(origin);
    v->destination = g_strdup(destination);
    v->aircraft = g_strdup(aircraft);
    v->airline = airline ? g_strdup(airline) : g_strdup("");
    v->tracking_url = tracking_url ? g_strdup(tracking_url) : g_strdup("");

    if (!v->flight_id || !v->departure || !v->actual_departure ||
        !v->arrival || !v->actual_arrival || !v->gate || !v->status ||
        !v->origin || !v->destination || !v->aircraft ||
        !v->airline || !v->tracking_url)
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
    free(v->departure);
    free(v->actual_departure);
    free(v->arrival);
    free(v->actual_arrival);
    free(v->gate);
    free(v->status);
    free(v->origin);
    free(v->destination);
    free(v->aircraft);
    free(v->airline);
    free(v->tracking_url);
    free(v);
}

const char *voo_obter_id(const voo_t *v)
{
    return v ? v->flight_id : NULL;
}

const char *voo_obter_departure(const voo_t *v)
{
    return v ? v->departure : NULL;
}

const char *voo_obter_actual_departure(const voo_t *v)
{
    return v ? v->actual_departure : NULL;
}

const char *voo_obter_arrival(const voo_t *v)
{
    return v ? v->arrival : NULL;
}

const char *voo_obter_actual_arrival(const voo_t *v)
{
    return v ? v->actual_arrival : NULL;
}

const char *voo_obter_gate(const voo_t *v)
{
    return v ? v->gate : NULL;
}

const char *voo_obter_status(const voo_t *v)
{
    return v ? v->status : NULL;
}

const char *voo_obter_origin(const voo_t *v)
{
    return v ? v->origin : NULL;
}

const char *voo_obter_destination(const voo_t *v)
{
    return v ? v->destination : NULL;
}

const char *voo_obter_aircraft(const voo_t *v)
{
    return v ? v->aircraft : NULL;
}

const char *voo_obter_airline(const voo_t *v)
{
    return v ? v->airline : NULL;
}

const char *voo_obter_tracking_url(const voo_t *v)
{
    return v ? v->tracking_url : NULL;
}