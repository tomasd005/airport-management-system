#include "voos.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct voo
 * @brief Estrutura que representa um voo.
 *
 * Contém informações sobre o voo, como identificador, origem, destino,
 * aeronave, companhia aérea, datas de partida, status e número de passageiros.
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
 * @enum voo_status_t
 * @brief Status possível de um voo.
 */
enum
{
    VOO_STATUS_ON_TIME = 0,
    VOO_STATUS_DELAYED = 1,
    VOO_STATUS_CANCELLED = 2
};

/** @brief Pool de strings internas para otimização de memória */
static GHashTable *intern_pool = NULL;

/**
 * @brief Interna uma string para reutilização em vários voos.
 * @param s String a ser internada
 * @return Ponteiro para a string internada
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
 * @brief Converte uma string de status para código interno.
 * @param status String do status
 * @return Código do status (VOO_STATUS_*)
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
 * @brief Converte o código de status interno para string.
 * @param status Código do status (VOO_STATUS_*)
 * @return String correspondente ao status
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
 * @param flight_id Identificador do voo
 * @param departure Data de partida prevista (string)
 * @param actual_departure Data de partida real (string)
 * @param arrival Data prevista de chegada
 * @param actual_arrival Data real de chegada
 * @param gate Portão de embarque
 * @param status Status do voo (string)
 * @param origin Aeroporto de origem
 * @param destination Aeroporto de destino
 * @param aircraft Aeronave
 * @param airline Companhia aérea
 * @param tracking_url URL de rastreamento
 * @return Ponteiro para o voo criado ou NULL em caso de erro
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
 * @brief Liberta a memória de um voo.
 * @param v Voo a destruir
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
 * @brief Obtém o identificador do voo.
 * @param v Ponteiro para o voo
 * @return Identificador do voo ou NULL
 */
const char *voo_obter_id(const voo_t *v) { return v ? v->flight_id : NULL; }

/**
 * @brief Obtém a data de partida prevista como string.
 * @param v Ponteiro para o voo
 * @return NULL (não implementado)
 */
const char *voo_obter_departure(const voo_t *v) { (void)v; return NULL; }

/**
 * @brief Obtém a data de partida real como string.
 * @param v Ponteiro para o voo
 * @return NULL (não implementado)
 */
const char *voo_obter_actual_departure(const voo_t *v) { (void)v; return NULL; }

/**
 * @brief Obtém a data prevista de chegada.
 * @param v Ponteiro para o voo
 * @return "N/A" (não implementado)
 */
const char *voo_obter_arrival(const voo_t *v) { return v ? "N/A" : NULL; }

/**
 * @brief Obtém a data real de chegada.
 * @param v Ponteiro para o voo
 * @return "N/A" (não implementado)
 */
const char *voo_obter_actual_arrival(const voo_t *v) { return v ? "N/A" : NULL; }

/**
 * @brief Obtém o portão de embarque.
 * @param v Ponteiro para o voo
 * @return String vazia
 */
const char *voo_obter_gate(const voo_t *v) { return v ? "" : NULL; }

/**
 * @brief Obtém o status do voo como string.
 * @param v Ponteiro para o voo
 * @return Status do voo
 */
const char *voo_obter_status(const voo_t *v) { return v ? status_to_str(v->status) : NULL; }

/**
 * @brief Obtém o aeroporto de origem do voo.
 * @param v Ponteiro para o voo
 * @return Aeroporto de origem
 */
const char *voo_obter_origin(const voo_t *v) { return v ? v->origin : NULL; }

/**
 * @brief Obtém o aeroporto de destino do voo.
 * @param v Ponteiro para o voo
 * @return Aeroporto de destino
 */
const char *voo_obter_destination(const voo_t *v) { return v ? v->destination : NULL; }

/**
 * @brief Obtém a aeronave do voo.
 * @param v Ponteiro para o voo
 * @return Aeronave
 */
const char *voo_obter_aircraft(const voo_t *v) { return v ? v->aircraft : NULL; }

/**
 * @brief Obtém a companhia aérea do voo.
 * @param v Ponteiro para o voo
 * @return Companhia aérea
 */
const char *voo_obter_airline(const voo_t *v) { return v ? v->airline : NULL; }

/**
 * @brief Obtém a URL de rastreamento do voo.
 * @param v Ponteiro para o voo
 * @return String vazia
 */
const char *voo_obter_tracking_url(const voo_t *v) { return v ? "" : NULL; }

/**
 * @brief Calcula o atraso do voo em minutos.
 * @param v Ponteiro para o voo
 * @return Atraso em minutos ou -1 se não disponível
 */
double voo_calcular_atraso_minutos(const voo_t *v)
{
    if (!v)
        return -1.0;
    return (v->atraso_min >= 0) ? (double)v->atraso_min : -1.0;
}

/**
 * @brief Obtém o dia de partida previsto.
 * @param v Ponteiro para o voo
 * @return Dia de partida ou -1 se não disponível
 */
int voo_obter_departure_dia(const voo_t *v) { return v ? v->dep_day : -1; }

/**
 * @brief Obtém o dia de partida real.
 * @param v Ponteiro para o voo
 * @return Dia real de partida ou -1 se não disponível
 */
int voo_obter_actual_departure_dia(const voo_t *v) { return v ? v->act_dep_day : -1; }

/**
 * @brief Obtém a semana do ano da partida prevista.
 * @param v Ponteiro para o voo
 * @return Semana do ano ou -1 se não disponível
 */
int voo_obter_semana(const voo_t *v) { return v ? v->semana : -1; }

/**
 * @brief Obtém o código do status do voo.
 * @param v Ponteiro para o voo
 * @return Código do status ou -1 se não disponível
 */
int voo_obter_status_codigo(const voo_t *v) { return v ? (int)v->status : -1; }

/**
 * @brief Obtém o número de passageiros do voo.
 * @param v Ponteiro para o voo
 * @return Número de passageiros ou 0 se não disponível
 */
int voo_obter_passageiros(const voo_t *v) { return v ? v->passageiros : 0; }

/**
 * @brief Incrementa o número de passageiros do voo.
 * @param v Ponteiro para o voo
 * @param delta Número de passageiros a adicionar
 */
void voo_incrementar_passageiros(voo_t *v, int delta)
{
    if (!v)
        return;
    v->passageiros += delta;
}

/**
 * @brief Descarta a aeronave associada ao voo.
 * @param v Ponteiro para o voo
 */
void voo_descartar_aircraft(voo_t *v)
{
    if (!v || !v->aircraft)
        return;
    free(v->aircraft);
    v->aircraft = NULL;
}

/**
 * @brief Descarta a companhia aérea associada ao voo.
 * @param v Ponteiro para o voo
 */
void voo_descartar_airline(voo_t *v)
{
    if (!v || !v->airline)
        return;
    free(v->airline);
    v->airline = NULL;
}

/**
 * @brief Destrói o pool de strings internas.
 */
void voo_intern_pool_destruir(void)
{
    if (!intern_pool)
        return;
    g_hash_table_destroy(intern_pool);
    intern_pool = NULL;
}
