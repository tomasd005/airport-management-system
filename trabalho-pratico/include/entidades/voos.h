#ifndef VOOS_H
#define VOOS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file voos.h
 * @brief Interface para manipulação de voos.
 *
 * Define a estrutura e funções para criar, destruir e obter informações
 * sobre voos, incluindo horários programados e reais, aeroporto de origem
 * e destino, aeronave, companhia aérea, status, atraso, número de passageiros
 * e outras informações relevantes.
 */

/**
 * @brief Estrutura opaca que representa um voo.
 */
typedef struct voo voo_t;

/**
 * @brief Estrutura com dados validados de um voo (para ingestão).
 */
typedef struct
{
    uint64_t key;
    uint16_t orig_idx;
    uint16_t dest_idx;
    int32_t dep_day;
    int32_t act_dep_day;
    int32_t atraso_min;
    int32_t semana;
    unsigned char status;
    const char *airline;
    const char *aircraft;
} voo_info_t;

/**
 * @brief Cria um novo voo.
 *
 * @param flight_id Identificador do voo (ex: "TP1234").
 * @param departure Horário de partida programado (formato "YYYY-MM-DD HH:MM").
 * @param actual_departure Horário real de partida (ou "N/A" se cancelado).
 * @param arrival Horário de chegada programado.
 * @param actual_arrival Horário real de chegada (ou "N/A" se cancelado).
 * @param gate Portão de embarque.
 * @param status Status do voo ("On Time", "Delayed", "Cancelled").
 * @param origin Código do aeroporto de origem (3 letras, ex: "LIS").
 * @param destination Código do aeroporto de destino.
 * @param aircraft Identificador da aeronave.
 * @param airline Nome da companhia aérea.
 * @param tracking_url URL de rastreamento do voo.
 * @return Ponteiro para o voo criado ou NULL em caso de erro.
 */
voo_t *voo_criar(const char *flight_id, const char *departure, const char *actual_departure, const char *arrival, const char *actual_arrival, const char *gate, const char *status, const char *origin, const char *destination, const char *aircraft, const char *airline, const char *tracking_url);

/**
 * @brief Cria um voo sem copiar strings (modo borrowed).
 *
 * Usado quando os dados permanecem válidos durante toda a execução.
 */

/**
 * @brief Destrói um voo e libera a memória associada.
 *
 * @param v Ponteiro para o voo a destruir.
 */
void voo_destruir(voo_t *v);

/**
 * @brief Cria uma estrutura de informação de voo (pré-parse).
 * @param flight_id ID do voo.
 * @param departure Data/hora de partida.
 * @param actual_departure Data/hora de partida real.
 * @param status Estado do voo.
 * @param origin Origem.
 * @param destination Destino.
 * @param aircraft ID da aeronave.
 * @param airline Companhia.
 * @return Estrutura criada, ou NULL em erro.
 */
voo_info_t *voo_info_criar(const char *flight_id,
                           const char *departure,
                           const char *actual_departure,
                           const char *status,
                           const char *origin,
                           const char *destination,
                           const char *aircraft,
                           const char *airline);

/**
 * @brief Destrói a estrutura de informação de voo.
 * @param info Estrutura a destruir (aceita NULL).
 */
void voo_info_destruir(voo_info_t *info);

/**
 * @brief Constrói um voo a partir de uma estrutura info.
 * @param info Estrutura info.
 * @return Voo criado, ou NULL em erro.
 */
voo_t *voo_criar_from_info(const voo_info_t *info);

/**
 * @brief Obtém chave compacta do voo.
 * @param v Voo.
 * @return Chave compacta.
 */
uint64_t voo_obter_key(const voo_t *v);

/**
 * @brief Obtém a data/hora de partida.
 * @param v Voo.
 * @return String constante.
 */
const char *voo_obter_departure(const voo_t *v);

/**
 * @brief Obtém a data/hora de partida real.
 * @param v Voo.
 * @return String constante.
 */
const char *voo_obter_actual_departure(const voo_t *v);

/**
 * @brief Obtém a data/hora de chegada.
 * @param v Voo.
 * @return String constante.
 */
const char *voo_obter_arrival(const voo_t *v);

/**
 * @brief Obtém a data/hora de chegada real.
 * @param v Voo.
 * @return String constante.
 */
const char *voo_obter_actual_arrival(const voo_t *v);

/**
 * @brief Obtém o gate.
 * @param v Voo.
 * @return String constante.
 */
const char *voo_obter_gate(const voo_t *v);

/**
 * @brief Obtém o status do voo.
 * @param v Voo.
 * @return String constante.
 */
const char *voo_obter_status(const voo_t *v);

/**
 * @brief Obtém a origem do voo.
 * @param v Voo.
 * @return String constante.
 */
const char *voo_obter_origin(const voo_t *v);

/**
 * @brief Obtém o destino do voo.
 * @param v Voo.
 * @return String constante.
 */
const char *voo_obter_destination(const voo_t *v);

/**
 * @brief Obtém o índice da origem (cache).
 * @param v Voo.
 * @return Índice da origem.
 */
int voo_obter_origin_idx(const voo_t *v);

/**
 * @brief Obtém o índice do destino (cache).
 * @param v Voo.
 * @return Índice do destino.
 */
int voo_obter_destination_idx(const voo_t *v);

/**
 * @brief Obtém o URL de tracking.
 * @param v Voo.
 * @return String constante.
 */
const char *voo_obter_tracking_url(const voo_t *v);

/**
 * @brief Calcula o atraso em minutos.
 * @param v Voo.
 * @return Atraso em minutos.
 */
double voo_calcular_atraso_minutos(const voo_t *v);

/**
 * @brief Obtém o dia (índice) da partida.
 * @param v Voo.
 * @return Dia em índice interno.
 */
int voo_obter_departure_dia(const voo_t *v);

/**
 * @brief Obtém o dia (índice) da partida real.
 * @param v Voo.
 * @return Dia em índice interno.
 */
int voo_obter_actual_departure_dia(const voo_t *v);

/**
 * @brief Obtém o índice da semana.
 * @param v Voo.
 * @return Semana em índice interno.
 */
int voo_obter_semana(const voo_t *v);

/**
 * @brief Obtém o código numérico do status.
 * @param v Voo.
 * @return Código do status.
 */
int voo_obter_status_codigo(const voo_t *v);

/**
 * @brief Obtém o número de passageiros agregados.
 * @param v Voo.
 * @return Contagem de passageiros.
 */
int voo_obter_passageiros(const voo_t *v);

/**
 * @brief Incrementa o número de passageiros.
 * @param v Voo.
 * @param delta Variação a aplicar.
 */
void voo_incrementar_passageiros(voo_t *v, int delta);

/**
 * @brief Liberta recursos associados ao interning de voos.
 */
void voo_intern_pool_destruir(void);

#endif
