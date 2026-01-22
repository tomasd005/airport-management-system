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
 * @brief Cria um novo voo com cópia dos campos de texto.
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
 * @brief Cria um novo voo sem copiar strings (modo borrowed).
 *
 * Deve ser usado apenas quando as strings permanecem válidas durante
 * toda a vida do voo (ex.: leitura via mmap).
 *
 * @param flight_id Identificador do voo.
 * @param departure Horário de partida programado.
 * @param actual_departure Horário real de partida.
 * @param arrival Horário de chegada programado.
 * @param actual_arrival Horário real de chegada.
 * @param gate Portão de embarque.
 * @param status Status do voo.
 * @param origin Código do aeroporto de origem.
 * @param destination Código do aeroporto de destino.
 * @param aircraft Identificador da aeronave.
 * @param airline Nome da companhia aérea.
 * @param tracking_url URL de rastreamento do voo.
 * @return Ponteiro para o voo criado ou NULL em caso de erro.
 */
voo_t *voo_criar_borrowed(const char *flight_id, const char *departure, const char *actual_departure, const char *arrival, const char *actual_arrival, const char *gate, const char *status, const char *origin, const char *destination, const char *aircraft, const char *airline, const char *tracking_url);

/**
 * @brief Destrói um voo e libera a memória associada.
 *
 * @param v Ponteiro para o voo a destruir.
 */
void voo_destruir(voo_t *v);

/**
 * @brief Obtém o identificador do voo.
 *
 * @param v Voo.
 * @return Identificador ou NULL se v for NULL.
 */
const char *voo_obter_id(const voo_t *v);

/**
 * @brief Obtém a partida programada do voo.
 *
 * @param v Voo.
 * @return Data/hora programada ou NULL se v for NULL.
 */
const char *voo_obter_departure(const voo_t *v);

/**
 * @brief Obtém a partida real do voo.
 *
 * @param v Voo.
 * @return Data/hora real ou NULL se v for NULL.
 */
const char *voo_obter_actual_departure(const voo_t *v);

/**
 * @brief Obtém a chegada programada do voo.
 *
 * @param v Voo.
 * @return Data/hora programada ou NULL se v for NULL.
 */
const char *voo_obter_arrival(const voo_t *v);

/**
 * @brief Obtém a chegada real do voo.
 *
 * @param v Voo.
 * @return Data/hora real ou NULL se v for NULL.
 */
const char *voo_obter_actual_arrival(const voo_t *v);

/**
 * @brief Obtém o portão de embarque do voo.
 *
 * @param v Voo.
 * @return Gate ou NULL se v for NULL.
 */
const char *voo_obter_gate(const voo_t *v);

/**
 * @brief Obtém o status do voo.
 *
 * @param v Voo.
 * @return Status textual ("On Time", "Delayed", "Cancelled").
 */
const char *voo_obter_status(const voo_t *v);

/**
 * @brief Obtém o código do aeroporto de origem.
 *
 * @param v Voo.
 * @return Código IATA ou NULL se v for NULL.
 */
const char *voo_obter_origin(const voo_t *v);

/**
 * @brief Obtém o índice numérico do aeroporto de origem.
 *
 * @param v Voo.
 * @return Índice (0..17575) ou -1 se inválido.
 */
int voo_obter_origin_idx(const voo_t *v);

/**
 * @brief Obtém o código do aeroporto de destino.
 *
 * @param v Voo.
 * @return Código IATA ou NULL se v for NULL.
 */
const char *voo_obter_destination(const voo_t *v);

/**
 * @brief Obtém o índice numérico do aeroporto de destino.
 *
 * @param v Voo.
 * @return Índice (0..17575) ou -1 se inválido.
 */
int voo_obter_destination_idx(const voo_t *v);

/**
 * @brief Obtém a aeronave do voo.
 *
 * @param v Voo.
 * @return Identificador da aeronave ou NULL se v for NULL.
 */
const char *voo_obter_aircraft(const voo_t *v);

/**
 * @brief Obtém a companhia aérea do voo.
 *
 * @param v Voo.
 * @return Nome da companhia ou NULL se v for NULL.
 */
const char *voo_obter_airline(const voo_t *v);

/**
 * @brief Obtém o URL de rastreamento do voo.
 *
 * @param v Voo.
 * @return URL de rastreamento ou NULL se v for NULL.
 */
const char *voo_obter_tracking_url(const voo_t *v);

/**
 * @brief Calcula o atraso do voo em minutos.
 *
 * @param v Voo.
 * @return Atraso em minutos, ou -1 se não aplicável.
 */
double voo_calcular_atraso_minutos(const voo_t *v);

/**
 * @brief Obtém o dia da partida programada (dias desde época).
 *
 * @param v Voo.
 * @return Dia numérico ou -1 se inválido.
 */
int voo_obter_departure_dia(const voo_t *v);

/**
 * @brief Obtém o dia da partida real (dias desde época).
 *
 * @param v Voo.
 * @return Dia numérico ou -1 se inválido.
 */
int voo_obter_actual_departure_dia(const voo_t *v);

/**
 * @brief Obtém a semana correspondente à partida programada.
 *
 * @param v Voo.
 * @return Identificador de semana.
 */
int voo_obter_semana(const voo_t *v);

/**
 * @brief Obtém o status do voo como código interno.
 *
 * @param v Voo.
 * @return Código do status (0/1/2).
 */
int voo_obter_status_codigo(const voo_t *v);

/**
 * @brief Obtém o número de passageiros do voo.
 *
 * @param v Voo.
 * @return Número de passageiros.
 */
int voo_obter_passageiros(const voo_t *v);

/**
 * @brief Incrementa o número de passageiros do voo.
 *
 * @param v Voo.
 * @param delta Valor a somar (pode ser negativo).
 */
void voo_incrementar_passageiros(voo_t *v, int delta);

/**
 * @brief Descarta a referência para a aeronave do voo.
 *
 * @param v Voo.
 */
void voo_descartar_aircraft(voo_t *v);

/**
 * @brief Descarta a referência para a companhia aérea do voo.
 *
 * @param v Voo.
 */
void voo_descartar_airline(voo_t *v);

/**
 * @brief Liberta o pool de strings internas de voos.
 */
void voo_intern_pool_destruir(void);

#endif
