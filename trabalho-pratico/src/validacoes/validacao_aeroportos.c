#include "validacao_aeroportos.h"
#include "validacao_comum.h"
#include "../../include/entidades/aeroportos.h"
#include "../../include/utils.h"
#include <string.h>
#include <ctype.h>

static inline gboolean codigo_valido(const char *codigo)
{
    if (!codigo || strlen(codigo) != 3 || contem_espacos(codigo))
        return FALSE;

    return g_ascii_isupper(codigo[0]) && g_ascii_isupper(codigo[1]) && g_ascii_isupper(codigo[2]);
}

static gboolean tipo_valido(const char *tipo)
{
    if (!tipo)
        return FALSE;

    static const char *tipos[] = {"small_airport", "medium_airport", "large_airport", "heliport", "seaplane_base"};

    for (int i = 0; i < 5; i++)
        if (strcmp(tipo, tipos[i]) == 0)
            return TRUE;

    return FALSE;
}

gpointer valida_aeroporto(char **colunas)
{
    if (!colunas)
        return NULL;

    for (int i = 0; i < 8; i++)
        if (!colunas[i])
            return NULL;

    for (int i = 0; i < 8; i++)
        utils_remove_aspas_somente(colunas[i]);

    if (contem_espacos(colunas[0]) || !codigo_valido(colunas[0]))
        return NULL;

    for (int i = 0; i < 8; i++)
        utils_trim(colunas[i]);

    double lat, lon;
    if (!coordenadas_validas(colunas[4], colunas[5], &lat, &lon) || !tipo_valido(colunas[7]))
        return NULL;

    if (!colunas[1] || !*colunas[1] || !colunas[2] || !*colunas[2] || !colunas[3] || !*colunas[3])
        return NULL;

    return aeroporto_criar(colunas[0], colunas[1], colunas[2], colunas[3], lat, lon, colunas[6], colunas[7]);
}