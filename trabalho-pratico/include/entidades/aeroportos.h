#ifndef AEROPORTOS_H
#define AEROPORTOS_H

#include <stdbool.h>

typedef struct aeroporto aeroporto_t;

aeroporto_t *aeroporto_criar(const char *codigo, const char *nome, const char *cidade, const char *pais, double latitude, double longitude, const char *icao, const char *tipo);

void aeroporto_destruir(aeroporto_t *a);

const char *aeroporto_obter_codigo(const aeroporto_t *a);
const char *aeroporto_obter_nome(const aeroporto_t *a);
const char *aeroporto_obter_cidade(const aeroporto_t *a);
const char *aeroporto_obter_pais(const aeroporto_t *a);
double aeroporto_obter_latitude(const aeroporto_t *a);
double aeroporto_obter_longitude(const aeroporto_t *a);
const char *aeroporto_obter_icao(const aeroporto_t *a);
const char *aeroporto_obter_tipo(const aeroporto_t *a);

#endif
