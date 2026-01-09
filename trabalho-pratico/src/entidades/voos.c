#include "voos.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct voo
 * @brief Representa um voo com origem, destino, aeronave, status e informações de horários.
 */
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
/**
 * @enum
 * @brief Códigos internos para status de voo.
 */
enum
{
    VOO_STATUS_ON_TIME = 0,  
    VOO_STATUS_DELAYED = 1,   
    VOO_STATUS_CANCELLED = 2
};

/** @brief Pool de strings para origem/destino, evitando duplicações. */
static GHashTable *intern_pool = NULL;

/**
 * @brief Interna uma string no pool global.
 * @param s String a ser internada
 * @return Ponteiro para string única no pool
 */
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

/**
 * @brief Converte string de status para código interno.
 * @param status String ("Cancelled", "Delayed", ou outro)
 * @return Código interno de status
 */
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

/**
 * @brief Converte código interno de status para string.
 * @param status Código interno
 * @return String correspondente
 */
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

/**
 * @brief Cria um novo voo.
 * @param flight_id ID do voo
 * @param departure Horário programado de partida
 * @param actual_departure Horário real de partida
 * @param arrival Horário programado de chegada
 * @param actual_arrival Horário real de chegada
 * @param gate Portão
 * @param status Status do voo ("On Time", "Delayed", "Cancelled")
 * @param origin Código do aeroporto de origem
 * @param destination Código do aeroporto de destino
 * @param aircraft Aeronave
 * @param airline Companhia aérea
 * @param tracking_url URL de rastreio 
 * @return Ponteiro para voo_t ou NULL em caso de erro
 */
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

/**
 * @brief Libera memória associada a um voo.
 * @param v Ponteiro para voo_t
 */
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

/**
 * @brief Obtém o ID do voo.
 */
const char *voo_obter_id(const voo_t *v) { return v ? v->flight_id : NULL; }

/**
 * @brief Obtém o aeroporto de origem.
 */
const char *voo_obter_origin(const voo_t *v) { return v ? v->origin : NULL; }

/**
 * @brief Obtém o aeroporto de destino.
 */
const char *voo_obter_destination(const voo_t *v) { return v ? v->destination : NULL; }

/**
 * @brief Obtém a aeronave.
 */
const char *voo_obter_aircraft(const voo_t *v) { return v ? v->aircraft : NULL; }

/**
 * @brief Obtém a companhia aérea.
 */
const char *voo_obter_airline(const voo_t *v) { return v ? v->airline : NULL; }

/**
 * @brief Obtém o status do voo como string.
 */
const char *voo_obter_status(const voo_t *v) { return v ? status_to_str(v->status) : NULL; }

/**
 * @brief Obtém o código interno do status do voo.
 */
int voo_obter_status_codigo(const voo_t *v) { return v ? (int)v->status : -1; }

/**
 * @brief Obtém o atraso em minutos.
 */
double voo_calcular_atraso_minutos(const voo_t *v) { return (v && v->atraso_min >= 0) ? (double)v->atraso_min : -1.0; }

/**
 * @brief Obtém o dia programado de partida.
 */
int voo_obter_departure_dia(const voo_t *v) { return v ? v->dep_day : -1; }

/**
 * @brief Obtém o dia real de partida.
 */
int voo_obter_actual_departure_dia(const voo_t *v) { return v ? v->act_dep_day : -1; }

/**
 * @brief Obtém a semana do voo.
 */
int voo_obter_semana(const voo_t *v) { return v ? v->semana : -1; }

/**
 * @brief Obtém número de passageiros.
 */
int voo_obter_passageiros(const voo_t *v) { return v ? v->passageiros : 0; }

/**
 * @brief Incrementa passageiros do voo.
 */
void voo_incrementar_passageiros(voo_t *v, int delta) { if (v) v->passageiros += delta; }

/**
 * @brief Descarta string aircraft.
 */
void voo_descartar_aircraft(voo_t *v) { if (v && v->aircraft) { free(v->aircraft); v->aircraft = NULL; } }

/**
 * @brief Descarta string airline.
 */
void voo_descartar_airline(voo_t *v) { if (v && v->airline) { free(v->airline); v->airline = NULL; } }

/**
 * @brief Destroi pool global de strings internadas.
 */
void voo_intern_pool_destruir(void)
{
    if (intern_pool)
    {
        g_hash_table_destroy(intern_pool);
        intern_pool = NULL;
    }
}
