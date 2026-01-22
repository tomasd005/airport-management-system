#ifndef AEROPORTOS_H
#define AEROPORTOS_H

#include <stdbool.h>

/**
 * @file aeroportos.h
 * @brief Interface para manipulação de aeroportos.
 *
 * Define a estrutura e funções para criar, destruir e obter informações
 * sobre aeroportos, incluindo código, localização e estatísticas de voos.
 */

/**
 * @brief Estrutura opaca que representa um aeroporto.
 */
typedef struct aeroporto aeroporto_t;

/**
 * @brief Cria um aeroporto.
 *
 * @param codigo Código IATA do aeroporto (3 letras maiúsculas).
 * @param nome Nome completo do aeroporto.
 * @param cidade Cidade onde o aeroporto se encontra.
 * @param pais País onde o aeroporto se encontra.
 * @param latitude Latitude do aeroporto em graus (-90 a 90).
 * @param longitude Longitude do aeroporto em graus (-180 a 180).
 * @param icao Código ICAO do aeroporto.
 * @param tipo Tipo do aeroporto (small_airport, medium_airport, large_airport, heliport, seaplane_base).
 * @return Ponteiro para o aeroporto criado ou NULL em caso de erro.
 */
aeroporto_t *aeroporto_criar(const char *codigo, const char *nome, const char *cidade, const char *pais, double latitude, double longitude, const char *icao, const char *tipo);

/**
 * @brief Destrói um aeroporto e libera a memória associada.
 *
 * @param a Ponteiro para o aeroporto a destruir.
 */
void aeroporto_destruir(aeroporto_t *a);

/**
 * @brief Obtém o código IATA do aeroporto.
 *
 * @param a Aeroporto.
 * @return Código IATA ou NULL se a for NULL.
 */
const char *aeroporto_obter_codigo(const aeroporto_t *a);

/**
 * @brief Obtém o nome do aeroporto.
 *
 * @param a Aeroporto.
 * @return Nome do aeroporto ou NULL se a for NULL.
 */
const char *aeroporto_obter_nome(const aeroporto_t *a);

/**
 * @brief Obtém a cidade do aeroporto.
 *
 * @param a Aeroporto.
 * @return Cidade do aeroporto ou NULL se a for NULL.
 */
const char *aeroporto_obter_cidade(const aeroporto_t *a);

/**
 * @brief Obtém o país do aeroporto.
 *
 * @param a Aeroporto.
 * @return País do aeroporto ou NULL se a for NULL.
 */
const char *aeroporto_obter_pais(const aeroporto_t *a);

/**
 * @brief Obtém a latitude do aeroporto.
 *
 * @param a Aeroporto.
 * @return Latitude em graus.
 */
double aeroporto_obter_latitude(const aeroporto_t *a);

/**
 * @brief Obtém a longitude do aeroporto.
 *
 * @param a Aeroporto.
 * @return Longitude em graus.
 */
double aeroporto_obter_longitude(const aeroporto_t *a);

/**
 * @brief Obtém o código ICAO do aeroporto.
 *
 * @param a Aeroporto.
 * @return Código ICAO ou NULL se a for NULL.
 */
const char *aeroporto_obter_icao(const aeroporto_t *a);

/**
 * @brief Obtém o tipo do aeroporto.
 *
 * @param a Aeroporto.
 * @return Tipo do aeroporto ou NULL se a for NULL.
 */
const char *aeroporto_obter_tipo(const aeroporto_t *a);

/**
 * @brief Obtém o total de partidas registadas no aeroporto.
 *
 * @param a Aeroporto.
 * @return Número de partidas.
 */
int aeroporto_obter_partidas(const aeroporto_t *a);

/**
 * @brief Obtém o total de chegadas registadas no aeroporto.
 *
 * @param a Aeroporto.
 * @return Número de chegadas.
 */
int aeroporto_obter_chegadas(const aeroporto_t *a);

/**
 * @brief Incrementa o contador de partidas do aeroporto.
 *
 * @param a Aeroporto.
 * @param delta Valor a somar (pode ser negativo).
 */
void aeroporto_incrementar_partidas(aeroporto_t *a, int delta);

/**
 * @brief Incrementa o contador de chegadas do aeroporto.
 *
 * @param a Aeroporto.
 * @param delta Valor a somar (pode ser negativo).
 */
void aeroporto_incrementar_chegadas(aeroporto_t *a, int delta);

#endif
