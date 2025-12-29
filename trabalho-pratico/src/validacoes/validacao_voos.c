#include "validacao_voos.h"
#include "validacao_comum.h"
#include "../../include/utils.h"
#include <string.h>
#include <ctype.h>

#define IDX_ID 0
#define IDX_DEP 1
#define IDX_ACT_DEP 2
#define IDX_ARR 3
#define IDX_ACT_ARR 4
#define IDX_GATE 5
#define IDX_STATUS 6
#define IDX_ORIG 7
#define IDX_DEST 8
#define IDX_AIR 9
#define IDX_AIRLINE 10
#define IDX_URL 11

voo_t *valida_voo(char **colunas)
{
    if (!colunas || !colunas[IDX_ID])
        return NULL;

    for (int i = 0; i <= IDX_URL; i++)
        if (colunas[i])
        {
            utils_remove_aspas_somente(colunas[i]);
            utils_trim(colunas[i]);
        }

    /* Campos obrigatórios */
    if (!*colunas[IDX_ID] ||
        !*colunas[IDX_ORIG] ||
        !*colunas[IDX_DEST] ||
        !*colunas[IDX_AIR] ||
        !*colunas[IDX_STATUS] ||
        !*colunas[IDX_DEP] ||
        !*colunas[IDX_ARR])
        return NULL;

    /* flight id: ccdddddd ou ccddddddd */
    size_t len = strlen(colunas[IDX_ID]);
    if (len != 7 && len != 8)
        return NULL;
    if (!validacao_flight_id(colunas[IDX_ID]))
        return NULL;

    /* códigos IATA */
    if (strlen(colunas[IDX_ORIG]) != 3 || strlen(colunas[IDX_DEST]) != 3)
        return NULL;
    for (int i = 0; i < 3; i++)
        if (!isupper(colunas[IDX_ORIG][i]) || !isupper(colunas[IDX_DEST][i]))
            return NULL;

    if (strcmp(colunas[IDX_ORIG], colunas[IDX_DEST]) == 0)
        return NULL;

    /* datetimes estimados */
    if (!validacao_datetime(colunas[IDX_DEP]) ||
        !validacao_datetime(colunas[IDX_ARR]))
        return NULL;

    if (strcmp(colunas[IDX_ARR], colunas[IDX_DEP]) < 0)
        return NULL;

    /* actual_* podem ser N/A */
    if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 &&
        !validacao_datetime(colunas[IDX_ACT_DEP]))
        return NULL;

    if (strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 &&
        !validacao_datetime(colunas[IDX_ACT_ARR]))
        return NULL;

    /* status */
    if (strcmp(colunas[IDX_STATUS], "On Time") != 0 &&
        strcmp(colunas[IDX_STATUS], "Delayed") != 0 &&
        strcmp(colunas[IDX_STATUS], "Cancelled") != 0)
        return NULL;

    if (strcmp(colunas[IDX_STATUS], "Cancelled") == 0)
    {
        if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 ||
            strcmp(colunas[IDX_ACT_ARR], "N/A") != 0)
            return NULL;
    }

    if (strcmp(colunas[IDX_STATUS], "Delayed") == 0)
    {
        if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 &&
            strcmp(colunas[IDX_ACT_DEP], colunas[IDX_DEP]) < 0)
            return NULL;

        if (strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 &&
            strcmp(colunas[IDX_ACT_ARR], colunas[IDX_ARR]) < 0)
            return NULL;
    }

    if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 &&
        strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 &&
        strcmp(colunas[IDX_ACT_ARR], colunas[IDX_ACT_DEP]) < 0)
        return NULL;

    return voo_criar(colunas[IDX_ID], colunas[IDX_DEP], colunas[IDX_ACT_DEP],
                     colunas[IDX_ARR], colunas[IDX_ACT_ARR], colunas[IDX_GATE],
                     colunas[IDX_STATUS], colunas[IDX_ORIG], colunas[IDX_DEST],
                     colunas[IDX_AIR], colunas[IDX_AIRLINE], colunas[IDX_URL]);
}