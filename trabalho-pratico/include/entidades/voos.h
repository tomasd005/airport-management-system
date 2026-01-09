#ifndef VOOS_H
#define VOOS_H

#include <stdbool.h>

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
 * @brief Destrói um voo e libera a memória associada.
 *
 * @param v Ponteiro para o voo a destruir.
 */
void voo_destruir(voo_t *v);

const char *voo_obter_id(const voo_t *v);
const char *voo_obter_departure(const voo_t *v);
const char *voo_obter_actual_departure(const voo_t *v);
const char *voo_obter_arrival(const voo_t *v);
const char *voo_obter_actual_arrival(const voo_t *v);
const char *voo_obter_gate(const voo_t *v);
const char *voo_obter_status(const voo_t *v);
const char *voo_obter_origin(const voo_t *v);
const char *voo_obter_destination(const voo_t *v);
const char *voo_obter_aircraft(const voo_t *v);
const char *voo_obter_airline(const voo_t *v);
const char *voo_obter_tracking_url(const voo_t *v);
double voo_calcular_atraso_minutos(const voo_t *v);
int voo_obter_departure_dia(const voo_t *v);
int voo_obter_actual_departure_dia(const voo_t *v);
int voo_obter_semana(const voo_t *v);
int voo_obter_status_codigo(const voo_t *v);
int voo_obter_passageiros(const voo_t *v);
void voo_incrementar_passageiros(voo_t *v, int delta);
void voo_descartar_aircraft(voo_t *v);
void voo_descartar_airline(voo_t *v);
void voo_intern_pool_destruir(void);

#endif
