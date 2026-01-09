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

const char *aeroporto_obter_codigo(const aeroporto_t *a);
const char *aeroporto_obter_nome(const aeroporto_t *a);
const char *aeroporto_obter_cidade(const aeroporto_t *a);
const char *aeroporto_obter_pais(const aeroporto_t *a);
double aeroporto_obter_latitude(const aeroporto_t *a);
double aeroporto_obter_longitude(const aeroporto_t *a);
const char *aeroporto_obter_icao(const aeroporto_t *a);
const char *aeroporto_obter_tipo(const aeroporto_t *a);
int aeroporto_obter_partidas(const aeroporto_t *a);
int aeroporto_obter_chegadas(const aeroporto_t *a);
void aeroporto_incrementar_partidas(aeroporto_t *a, int delta);
void aeroporto_incrementar_chegadas(aeroporto_t *a, int delta);

#endif
