#include "voos.h"
#include "parsers/parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <time.h>

struct voo
{
    char *flight_id;
    char *departure;
    char *actual_departure;
    char *status;
    char *origin;
    char *destination;
    char *aircraft;
    char *airline;
};

static time_t datetime_para_time(const char *datetime);

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
    v->departure = g_strdup(departure);
    v->actual_departure = actual_departure ? g_strdup(actual_departure) : g_strdup("N/A");
    v->status = g_strdup(status);
    v->origin = g_strdup(origin);
    v->destination = g_strdup(destination);
    v->aircraft = g_strdup(aircraft);
    v->airline = airline ? g_strdup(airline) : g_strdup("");

    if (!v->flight_id || !v->departure || !v->actual_departure ||
        !v->status || !v->origin || !v->destination || !v->aircraft || !v->airline)
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
    free(v->status);
    free(v->origin);
    free(v->destination);
    free(v->aircraft);
    free(v->airline);
    free(v);
}

const char *voo_obter_id(const voo_t *v) { return v ? v->flight_id : NULL; }
const char *voo_obter_departure(const voo_t *v) { return v ? v->departure : NULL; }
const char *voo_obter_actual_departure(const voo_t *v) { return v ? v->actual_departure : NULL; }
const char *voo_obter_arrival(const voo_t *v) { return v ? "N/A" : NULL; }
const char *voo_obter_actual_arrival(const voo_t *v) { return v ? "N/A" : NULL; }
const char *voo_obter_gate(const voo_t *v) { return v ? "" : NULL; }
const char *voo_obter_status(const voo_t *v) { return v ? v->status : NULL; }
const char *voo_obter_origin(const voo_t *v) { return v ? v->origin : NULL; }
const char *voo_obter_destination(const voo_t *v) { return v ? v->destination : NULL; }
const char *voo_obter_aircraft(const voo_t *v) { return v ? v->aircraft : NULL; }
const char *voo_obter_airline(const voo_t *v) { return v ? v->airline : NULL; }
const char *voo_obter_tracking_url(const voo_t *v) { return v ? "" : NULL; }

static time_t datetime_para_time(const char *datetime)
{
    if (!datetime || strlen(datetime) < 16)
        return (time_t)-1;

    struct tm t = {0};
    int ano, mes, dia, hora, min;

    if (sscanf(datetime, "%d-%d-%d %d:%d", &ano, &mes, &dia, &hora, &min) == 5)
    {
        t.tm_year = ano - 1900;
        t.tm_mon = mes - 1;
        t.tm_mday = dia;
        t.tm_hour = hora;
        t.tm_min = min;
        t.tm_isdst = -1;
        return mktime(&t);
    }

    if (sscanf(datetime, "%d/%d/%d %d:%d", &ano, &mes, &dia, &hora, &min) == 5)
    {
        t.tm_year = ano - 1900;
        t.tm_mon = mes - 1;
        t.tm_mday = dia;
        t.tm_hour = hora;
        t.tm_min = min;
        t.tm_isdst = -1;
        return mktime(&t);
    }

    return (time_t)-1;
}

double voo_calcular_atraso_minutos(const voo_t *v)
{
    if (!v)
        return -1.0;

    if (!v->departure || !v->actual_departure)
        return -1.0;

    if (strcmp(v->actual_departure, "N/A") == 0)
        return -1.0;

    time_t dep = datetime_para_time(v->departure);
    time_t act = datetime_para_time(v->actual_departure);
    if (dep == (time_t)-1 || act == (time_t)-1)
        return -1.0;

    double diff = difftime(act, dep) / 60.0;
    return (diff < 0.0) ? -1.0 : diff;
}
