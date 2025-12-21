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

    // Remove aspas ANTES de qualquer validação
    for (int i = 0; i <= IDX_URL; i++)
        if (colunas[i])
            utils_remove_aspas_somente(colunas[i]);

    // ✅ VALIDAÇÃO: flight_id não pode ter espaços
    if (contem_espacos(colunas[IDX_ID]))
        return NULL;

    if (!validacao_flight_id(colunas[IDX_ID]))
        return NULL;

    // ✅ VALIDAÇÃO: origin não pode ter espaços
    if (!colunas[IDX_ORIG] || contem_espacos(colunas[IDX_ORIG]))
        return NULL;
    
    // ✅ VALIDAÇÃO: destination não pode ter espaços
    if (!colunas[IDX_DEST] || contem_espacos(colunas[IDX_DEST]))
        return NULL;

    // ✅ NOVO: aircraft não pode ter espaços
    if (!colunas[IDX_AIR] || contem_espacos(colunas[IDX_AIR]))
        return NULL;

    // Trim DEPOIS de validar espaços internos
    for (int i = 0; i <= IDX_URL; i++)
        if (colunas[i])
            utils_trim(colunas[i]);

    // Valida campos obrigatórios não vazios
    if (!colunas[IDX_ORIG] || !*colunas[IDX_ORIG])
        return NULL;
    if (!colunas[IDX_DEST] || !*colunas[IDX_DEST])
        return NULL;
    if (!colunas[IDX_AIR] || !*colunas[IDX_AIR])
        return NULL;
    if (!colunas[IDX_STATUS] || !*colunas[IDX_STATUS])
        return NULL;

    // Valida comprimento dos códigos de aeroporto
    if (strlen(colunas[IDX_ORIG]) != 3 || strlen(colunas[IDX_DEST]) != 3)
        return NULL;

    // Valida que são letras maiúsculas
    for (int i = 0; i < 3; i++)
    {
        if (!isupper((unsigned char)colunas[IDX_ORIG][i]))
            return NULL;
        if (!isupper((unsigned char)colunas[IDX_DEST][i]))
            return NULL;
    }

    // Origin != Destination
    if (strcmp(colunas[IDX_ORIG], colunas[IDX_DEST]) == 0)
        return NULL;

    // Valida datetimes
    if (!validacao_datetime(colunas[IDX_DEP]))
        return NULL;
    if (!validacao_datetime(colunas[IDX_ARR]))
        return NULL;

    // Arrival >= Departure
    if (strcmp(colunas[IDX_ARR], colunas[IDX_DEP]) < 0)
        return NULL;

    // Valida actual_departure (pode ser "N/A")
    if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0)
        if (!validacao_datetime(colunas[IDX_ACT_DEP]))
            return NULL;

    // Valida actual_arrival (pode ser "N/A")
    if (strcmp(colunas[IDX_ACT_ARR], "N/A") != 0)
        if (!validacao_datetime(colunas[IDX_ACT_ARR]))
            return NULL;

    // Valida status
    if (strcmp(colunas[IDX_STATUS], "On Time") != 0 &&
        strcmp(colunas[IDX_STATUS], "Delayed") != 0 &&
        strcmp(colunas[IDX_STATUS], "Cancelled") != 0)
        return NULL;

    // REGRA: Cancelled → actual_* = "N/A"
    if (strcmp(colunas[IDX_STATUS], "Cancelled") == 0)
    {
        if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 ||
            strcmp(colunas[IDX_ACT_ARR], "N/A") != 0)
            return NULL;
    }

    // REGRA: Delayed → actual_* >= planned
    if (strcmp(colunas[IDX_STATUS], "Delayed") == 0)
    {
        if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0)
            if (strcmp(colunas[IDX_ACT_DEP], colunas[IDX_DEP]) < 0)
                return NULL;

        if (strcmp(colunas[IDX_ACT_ARR], "N/A") != 0)
            if (strcmp(colunas[IDX_ACT_ARR], colunas[IDX_ARR]) < 0)
                return NULL;
    }

    // REGRA: actual_arrival >= actual_departure (se ambos não "N/A")
    if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 &&
        strcmp(colunas[IDX_ACT_ARR], "N/A") != 0)
    {
        if (strcmp(colunas[IDX_ACT_ARR], colunas[IDX_ACT_DEP]) < 0)
            return NULL;
    }

    return voo_criar(colunas[IDX_ID], colunas[IDX_DEP], colunas[IDX_ACT_DEP],
                     colunas[IDX_ARR], colunas[IDX_ACT_ARR], colunas[IDX_GATE],
                     colunas[IDX_STATUS], colunas[IDX_ORIG], colunas[IDX_DEST],
                     colunas[IDX_AIR], colunas[IDX_AIRLINE], colunas[IDX_URL]);
}