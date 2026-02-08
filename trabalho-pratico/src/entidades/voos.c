#include "voos.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @struct voo
 * @brief Estrutura que representa um voo.
 *
 * Contém informações sobre o voo, como identificador, origem, destino,
 * aeronave, companhia aérea, datas de partida, status e número de passageiros.
 */
struct voo
{
    int32_t act_dep_day;
    int32_t atraso_min;
    int32_t semana;
    uint32_t passageiros;
    uint16_t orig_idx;
    uint16_t dest_idx;
    unsigned char status;
};

struct voo_pool
{
    size_t block_capacity;
    voo_t **blocks;
    uint32_t blocks_count;
    uint32_t blocks_capacity;
    uint32_t count;
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

static inline void voo_preencher_de_info(voo_t *v, const voo_info_t *info)
{
    v->orig_idx = info->orig_idx;
    v->dest_idx = info->dest_idx;
    v->act_dep_day = info->act_dep_day;
    v->atraso_min = info->atraso_min;
    v->semana = info->semana;
    v->status = info->status;
    v->passageiros = 0;
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

    (void)arrival;
    (void)actual_arrival;
    (void)gate;
    (void)tracking_url;
    (void)airline;
    (void)aircraft;

    voo_info_t *info = voo_info_criar(flight_id, departure, actual_departure,
                                      status, origin, destination, aircraft, airline);
    if (!info)
        return NULL;

    voo_t *v = malloc(sizeof(voo_t));
    if (!v)
    {
        voo_info_destruir(info);
        return NULL;
    }

    voo_preencher_de_info(v, info);

    voo_info_destruir(info);
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

    free(v);
}

voo_pool_t *voo_pool_criar(size_t block_capacity)
{
    if (block_capacity < 1024)
        block_capacity = 1024;

    voo_pool_t *pool = malloc(sizeof(*pool));
    if (!pool)
        return NULL;

    pool->block_capacity = block_capacity;
    pool->blocks = NULL;
    pool->blocks_count = 0;
    pool->blocks_capacity = 0;
    pool->count = 0;
    return pool;
}

void voo_pool_destruir(voo_pool_t *pool)
{
    if (!pool)
        return;

    for (uint32_t i = 0; i < pool->blocks_count; i++)
        free(pool->blocks[i]);
    free(pool->blocks);
    free(pool);
}

static int voo_pool_garantir_bloco(voo_pool_t *pool, uint32_t block_idx)
{
    if (block_idx < pool->blocks_count)
        return 1;

    if (block_idx >= pool->blocks_capacity)
    {
        uint32_t new_cap = pool->blocks_capacity ? (pool->blocks_capacity * 2u) : 16u;
        while (new_cap <= block_idx)
            new_cap *= 2u;

        voo_t **novo = realloc(pool->blocks, (size_t)new_cap * sizeof(voo_t *));
        if (!novo)
            return 0;
        for (uint32_t i = pool->blocks_capacity; i < new_cap; i++)
            novo[i] = NULL;
        pool->blocks = novo;
        pool->blocks_capacity = new_cap;
    }

    while (pool->blocks_count <= block_idx)
    {
        voo_t *bloco = malloc(pool->block_capacity * sizeof(voo_t));
        if (!bloco)
            return 0;
        pool->blocks[pool->blocks_count++] = bloco;
    }

    return 1;
}

voo_t *voo_pool_obter(const voo_pool_t *pool, uint32_t id)
{
    if (!pool)
        return NULL;
    uint32_t block_idx = id / (uint32_t)pool->block_capacity;
    uint32_t off = id % (uint32_t)pool->block_capacity;
    if (block_idx >= pool->blocks_count)
        return NULL;
    return &pool->blocks[block_idx][off];
}

voo_t *voo_pool_criar_from_info_com_id(voo_pool_t *pool, const voo_info_t *info, uint32_t *out_id)
{
    if (!pool || !info || info->key == 0 || !out_id)
        return NULL;

    uint32_t id = pool->count++;
    uint32_t block_idx = id / (uint32_t)pool->block_capacity;
    uint32_t off = id % (uint32_t)pool->block_capacity;
    if (!voo_pool_garantir_bloco(pool, block_idx))
    {
        pool->count--;
        return NULL;
    }

    voo_t *v = &pool->blocks[block_idx][off];
    voo_preencher_de_info(v, info);
    *out_id = id;
    return v;
}

voo_t *voo_pool_criar_from_info(voo_pool_t *pool, const voo_info_t *info)
{
    uint32_t id = 0;
    return voo_pool_criar_from_info_com_id(pool, info, &id);
}

/**
 * @brief Obtém o identificador do voo.
 * @param v Ponteiro para o voo
 * @return Identificador do voo ou NULL
 */
uint64_t voo_obter_key(const voo_t *v) { (void)v; return 0; }

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
const char *voo_obter_origin(const voo_t *v)
{
    if (!v)
        return NULL;
    return utils_aeroporto_codigo_const(voo_obter_origin_idx(v));
}

/**
 * @brief Obtém o aeroporto de destino do voo.
 * @param v Ponteiro para o voo
 * @return Aeroporto de destino
 */
const char *voo_obter_destination(const voo_t *v)
{
    if (!v)
        return NULL;
    return utils_aeroporto_codigo_const(voo_obter_destination_idx(v));
}

int voo_obter_origin_idx(const voo_t *v)
{
    if (!v)
        return -1;
    return (v->orig_idx == 0xFFFF) ? -1 : (int)v->orig_idx;
}

int voo_obter_destination_idx(const voo_t *v)
{
    if (!v)
        return -1;
    return (v->dest_idx == 0xFFFF) ? -1 : (int)v->dest_idx;
}

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
int voo_obter_departure_dia(const voo_t *v) { (void)v; return -1; }

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
void voo_intern_pool_destruir(void) {}

voo_info_t *voo_info_criar(const char *flight_id,
                           const char *departure,
                           const char *actual_departure,
                           const char *status,
                           const char *origin,
                           const char *destination,
                           const char *aircraft,
                           const char *airline)
{
    if (!flight_id || !departure || !status || !origin || !destination || !aircraft)
        return NULL;

    voo_info_t *info = malloc(sizeof(voo_info_t));
    if (!info)
        return NULL;

    if (!utils_flight_id_key(flight_id, &info->key))
    {
        free(info);
        return NULL;
    }

    info->dep_day = utils_parse_datetime_to_day_fast(departure);
    info->act_dep_day = utils_parse_datetime_to_day_fast(actual_departure);
    info->semana = utils_week_from_day(info->dep_day);
    info->status = status_from_str(status);
    info->atraso_min = -1;

    if (actual_departure && strcmp(actual_departure, "N/A") != 0)
    {
        int dep_min = utils_parse_datetime_to_minutes_fast(departure);
        int act_min = utils_parse_datetime_to_minutes_fast(actual_departure);
        if (dep_min >= 0 && act_min >= 0 && act_min >= dep_min)
            info->atraso_min = act_min - dep_min;
    }

    int orig_idx = utils_aeroporto_index(origin);
    int dest_idx = utils_aeroporto_index(destination);
    info->orig_idx = (orig_idx >= 0) ? (uint16_t)orig_idx : 0xFFFF;
    info->dest_idx = (dest_idx >= 0) ? (uint16_t)dest_idx : 0xFFFF;
    info->aircraft = aircraft;
    info->airline = airline;
    return info;
}

void voo_info_destruir(voo_info_t *info)
{
    free(info);
}

voo_t *voo_criar_from_info(const voo_info_t *info)
{
    if (!info || info->key == 0)
        return NULL;

    voo_t *v = malloc(sizeof(voo_t));
    if (!v)
        return NULL;

    voo_preencher_de_info(v, info);
    return v;
}
