#include "validacao_voos.h"
#include "validacao_comum.h"
#include "../../include/utils.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/** Índices das colunas do CSV de voos */
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

/**
 * @brief Valida um voo a partir de um array de colunas CSV.
 *
 * Valida todos os campos obrigatórios, bem como regras lógicas:
 * - Flight ID deve estar no formato correto
 * - Origem e destino devem ser diferentes e ter códigos de 3 letras maiúsculas
 * - Datas previstas e reais devem estar corretas e consistentes
 * - Status do voo deve ser "On Time", "Delayed" ou "Cancelled"
 * - Se cancelado, datas reais devem ser "N/A"
 * - Se atrasado, datas reais >= datas previstas
 *
 * @param colunas Array de strings com os campos do voo
 * @return Ponteiro para voo_t criado se válido, NULL caso contrário
 */
voo_t *valida_voo(char **colunas)
{
    if (!colunas || !colunas[IDX_ID])
        return NULL;

    for (int i = 0; i <= IDX_URL; i++)
        if (colunas[i])
            utils_remove_aspas_somente(colunas[i]);

    if (colunas[IDX_ID] && contem_espacos(colunas[IDX_ID]))
        return NULL;

    if (!colunas[IDX_ID] || !validacao_flight_id(colunas[IDX_ID]))
        return NULL;

    if ((colunas[IDX_ORIG] && contem_espacos(colunas[IDX_ORIG])) ||
        (colunas[IDX_DEST] && contem_espacos(colunas[IDX_DEST])) ||
        (colunas[IDX_AIR] && contem_espacos(colunas[IDX_AIR])))
        return NULL;

    for (int i = 0; i <= IDX_URL; i++)
        if (colunas[i])
            utils_trim(colunas[i]);

    if (!*colunas[IDX_ID] || !*colunas[IDX_ORIG] || !*colunas[IDX_DEST] ||
        !*colunas[IDX_AIR] || !*colunas[IDX_STATUS] || !*colunas[IDX_DEP] || !*colunas[IDX_ARR])
        return NULL;

    if (strlen(colunas[IDX_ORIG]) != 3 || strlen(colunas[IDX_DEST]) != 3 ||
        !isupper(colunas[IDX_ORIG][0]) || !isupper(colunas[IDX_ORIG][1]) || !isupper(colunas[IDX_ORIG][2]) ||
        !isupper(colunas[IDX_DEST][0]) || !isupper(colunas[IDX_DEST][1]) || !isupper(colunas[IDX_DEST][2]) ||
        strcmp(colunas[IDX_ORIG], colunas[IDX_DEST]) == 0 ||
        !validacao_datetime(colunas[IDX_DEP]) || !validacao_datetime(colunas[IDX_ARR]))
        return NULL;

    if (comparar_datetime(colunas[IDX_ARR], colunas[IDX_DEP]) < 0)
        return NULL;

    if (strcmp(colunas[IDX_STATUS], "On Time") != 0 &&
        strcmp(colunas[IDX_STATUS], "Delayed") != 0 &&
        strcmp(colunas[IDX_STATUS], "Cancelled") != 0)
        return NULL;

    if (strcmp(colunas[IDX_STATUS], "Cancelled") == 0 &&
        (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 || strcmp(colunas[IDX_ACT_ARR], "N/A") != 0))
        return NULL;

    if (strcmp(colunas[IDX_STATUS], "Cancelled") != 0)
    {
        if (!validacao_datetime(colunas[IDX_ACT_DEP]) || !validacao_datetime(colunas[IDX_ACT_ARR]))
            return NULL;
    }

    if (strcmp(colunas[IDX_STATUS], "Delayed") == 0)
    {
        if (comparar_datetime(colunas[IDX_ACT_DEP], colunas[IDX_DEP]) < 0)
            return NULL;
        if (comparar_datetime(colunas[IDX_ACT_ARR], colunas[IDX_ARR]) < 0)
            return NULL;
    }

    if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 && strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 &&
        comparar_datetime(colunas[IDX_ACT_ARR], colunas[IDX_ACT_DEP]) < 0)
        return NULL;

    return voo_criar(colunas[IDX_ID], colunas[IDX_DEP], colunas[IDX_ACT_DEP],
                     colunas[IDX_ARR], colunas[IDX_ACT_ARR], colunas[IDX_GATE],
                     colunas[IDX_STATUS], colunas[IDX_ORIG], colunas[IDX_DEST],
                     colunas[IDX_AIR], colunas[IDX_AIRLINE], colunas[IDX_URL]);
}
