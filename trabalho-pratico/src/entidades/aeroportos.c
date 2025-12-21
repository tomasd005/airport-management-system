#include "aeroportos.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>

struct aeroporto
{
    char *codigo;
    char *nome;
    char *cidade;
    char *pais;
    double latitude;
    double longitude;
    char *icao;
    char *tipo;
};

aeroporto_t *aeroporto_criar(const char *codigo, const char *nome, const char *cidade,
                             const char *pais, double latitude, double longitude,
                             const char *icao, const char *tipo)
{
    if (!codigo || !nome || !cidade || !pais || !icao || !tipo)
        return NULL;

    aeroporto_t *a = malloc(sizeof(aeroporto_t));
    if (!a)
        return NULL;

    a->codigo = g_strdup(codigo);
    a->nome = g_strdup(nome);
    a->cidade = g_strdup(cidade);
    a->pais = g_strdup(pais);
    a->latitude = latitude;
    a->longitude = longitude;
    a->icao = g_strdup(icao);
    a->tipo = g_strdup(tipo);

    if (!a->codigo || !a->nome || !a->cidade || !a->pais || !a->icao || !a->tipo)
    {
        aeroporto_destruir(a);
        return NULL;
    }

    return a;
}

void aeroporto_destruir(aeroporto_t *a)
{
    if (!a)
        return;

    free(a->codigo);
    free(a->nome);
    free(a->cidade);
    free(a->pais);
    free(a->icao);
    free(a->tipo);
    free(a);
}

const char *aeroporto_obter_codigo(const aeroporto_t *a)
{
    return a ? a->codigo : NULL;
}

const char *aeroporto_obter_nome(const aeroporto_t *a)
{
    return a ? a->nome : NULL;
}

const char *aeroporto_obter_cidade(const aeroporto_t *a)
{
    return a ? a->cidade : NULL;
}

const char *aeroporto_obter_pais(const aeroporto_t *a)
{
    return a ? a->pais : NULL;
}

double aeroporto_obter_latitude(const aeroporto_t *a)
{
    return a ? a->latitude : 0.0;
}

double aeroporto_obter_longitude(const aeroporto_t *a)
{
    return a ? a->longitude : 0.0;
}

const char *aeroporto_obter_icao(const aeroporto_t *a)
{
    return a ? a->icao : NULL;
}

const char *aeroporto_obter_tipo(const aeroporto_t *a)
{
    return a ? a->tipo : NULL;
}