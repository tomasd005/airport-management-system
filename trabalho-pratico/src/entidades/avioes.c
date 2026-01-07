#include "avioes.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>

struct aviao
{
    char *identificador;
    char *fabricante;
    char *modelo;
};

aviao_t *aviao_criar(const char *identificador, const char *fabricante,
                     const char *modelo, int ano, int capacidade, int alcance_km)
{
    if (!identificador || !fabricante || !modelo)
        return NULL;

    aviao_t *a = malloc(sizeof(aviao_t));
    if (!a)
        return NULL;

    a->identificador = g_strdup(identificador);
    a->fabricante = g_strdup(fabricante);
    a->modelo = g_strdup(modelo);

    if (!a->identificador || !a->fabricante || !a->modelo)
    {
        aviao_destruir(a);
        return NULL;
    }

    return a;
}

void aviao_destruir(aviao_t *a)
{
    if (!a)
        return;

    free(a->identificador);
    free(a->fabricante);
    free(a->modelo);
    free(a);
}

const char *aviao_obter_identificador(const aviao_t *a)
{
    return a ? a->identificador : NULL;
}

const char *aviao_obter_fabricante(const aviao_t *a)
{
    return a ? a->fabricante : NULL;
}

const char *aviao_obter_modelo(const aviao_t *a)
{
    return a ? a->modelo : NULL;
}

int aviao_obter_ano(const aviao_t *a)
{
    return 0;
}

int aviao_obter_capacidade(const aviao_t *a)
{
    return 0;
}

int aviao_obter_alcance_km(const aviao_t *a)
{
    return 0;
}