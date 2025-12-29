#include "validacao_voos.h"
#include "validacao_comum.h"
#include "../../include/utils.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

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
    LOG_DEBUG("Entrou em valida_voo");

    if (!colunas || !colunas[IDX_ID])
    {
        LOG_DEBUG("colunas ou IDX_ID é NULL");
        return NULL;
    }

    for (int i = 0; i <= IDX_URL; i++)
    {
        if (colunas[i])
        {
            utils_remove_aspas_somente(colunas[i]);
            utils_trim(colunas[i]);
            LOG_DEBUG("Coluna %d após limpeza: '%s'", i, colunas[i]);
        }
    }

    /* Campos obrigatórios */
    if (!*colunas[IDX_ID] || !*colunas[IDX_ORIG] || !*colunas[IDX_DEST] ||
        !*colunas[IDX_AIR] || !*colunas[IDX_STATUS] || !*colunas[IDX_DEP] || !*colunas[IDX_ARR])
    {
        LOG_DEBUG("Falha nos campos obrigatórios");
        return NULL;
    }

    /* flight id: ccdddddd ou ccddddddd */
    size_t len = strlen(colunas[IDX_ID]);
    if (len != 7 && len != 8)
    {
        LOG_DEBUG("IDX_ID '%s' tamanho inválido: %zu", colunas[IDX_ID], len);
        return NULL;
    }
    if (!validacao_flight_id(colunas[IDX_ID]))
    {
        LOG_DEBUG("IDX_ID '%s' falhou validacao_flight_id", colunas[IDX_ID]);
        return NULL;
    }

    /* códigos IATA */
    if (strlen(colunas[IDX_ORIG]) != 3 || strlen(colunas[IDX_DEST]) != 3)
    {
        LOG_DEBUG("IATA codes inválidos: ORIG='%s', DEST='%s'", colunas[IDX_ORIG], colunas[IDX_DEST]);
        return NULL;
    }
    for (int i = 0; i < 3; i++)
    {
        if (!isupper(colunas[IDX_ORIG][i]) || !isupper(colunas[IDX_DEST][i]))
        {
            LOG_DEBUG("IATA code não maiúsculo: ORIG='%s', DEST='%s'", colunas[IDX_ORIG], colunas[IDX_DEST]);
            return NULL;
        }
    }

    if (strcmp(colunas[IDX_ORIG], colunas[IDX_DEST]) == 0)
    {
        LOG_DEBUG("ORIG e DEST iguais: '%s'", colunas[IDX_ORIG]);
        return NULL;
    }

    /* datetimes estimados */
    if (!validacao_datetime(colunas[IDX_DEP]) || !validacao_datetime(colunas[IDX_ARR]))
    {
        LOG_DEBUG("Falha validacao_datetime: DEP='%s', ARR='%s'", colunas[IDX_DEP], colunas[IDX_ARR]);
        return NULL;
    }

    if (strcmp(colunas[IDX_ARR], colunas[IDX_DEP]) < 0)
    {
        LOG_DEBUG("ARR < DEP: DEP='%s', ARR='%s'", colunas[IDX_DEP], colunas[IDX_ARR]);
        return NULL;
    }

    /* actual_* podem ser N/A */
    if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 && !validacao_datetime(colunas[IDX_ACT_DEP]))
    {
        LOG_DEBUG("ACT_DEP inválido: '%s'", colunas[IDX_ACT_DEP]);
        return NULL;
    }

    if (strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 && !validacao_datetime(colunas[IDX_ACT_ARR]))
    {
        LOG_DEBUG("ACT_ARR inválido: '%s'", colunas[IDX_ACT_ARR]);
        return NULL;
    }

    /* status */
    if (strcmp(colunas[IDX_STATUS], "On Time") != 0 &&
        strcmp(colunas[IDX_STATUS], "Delayed") != 0 &&
        strcmp(colunas[IDX_STATUS], "Cancelled") != 0)
    {
        LOG_DEBUG("Status inválido: '%s'", colunas[IDX_STATUS]);
        return NULL;
    }

    if (strcmp(colunas[IDX_STATUS], "Cancelled") == 0)
    {
        if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 || strcmp(colunas[IDX_ACT_ARR], "N/A") != 0)
        {
            LOG_DEBUG("Cancelled mas ACT_DEP/ARR != N/A: ACT_DEP='%s', ACT_ARR='%s'", colunas[IDX_ACT_DEP], colunas[IDX_ACT_ARR]);
            return NULL;
        }
    }

    if (strcmp(colunas[IDX_STATUS], "Delayed") == 0)
    {
        if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 && strcmp(colunas[IDX_ACT_DEP], colunas[IDX_DEP]) < 0)
        {
            LOG_DEBUG("Delayed mas ACT_DEP < DEP: ACT_DEP='%s', DEP='%s'", colunas[IDX_ACT_DEP], colunas[IDX_DEP]);
            return NULL;
        }

        if (strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 && strcmp(colunas[IDX_ACT_ARR], colunas[IDX_ARR]) < 0)
        {
            LOG_DEBUG("Delayed mas ACT_ARR < ARR: ACT_ARR='%s', ARR='%s'", colunas[IDX_ACT_ARR], colunas[IDX_ARR]);
            return NULL;
        }
    }

    if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 && strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 &&
        strcmp(colunas[IDX_ACT_ARR], colunas[IDX_ACT_DEP]) < 0)
    {
        LOG_DEBUG("ACT_ARR < ACT_DEP: ACT_DEP='%s', ACT_ARR='%s'", colunas[IDX_ACT_DEP], colunas[IDX_ACT_ARR]);
        return NULL;
    }

    LOG_DEBUG("Validação concluída com sucesso, criando voo_t");
    return voo_criar(colunas[IDX_ID], colunas[IDX_DEP], colunas[IDX_ACT_DEP],
                     colunas[IDX_ARR], colunas[IDX_ACT_ARR], colunas[IDX_GATE],
                     colunas[IDX_STATUS], colunas[IDX_ORIG], colunas[IDX_DEST],
                     colunas[IDX_AIR], colunas[IDX_AIRLINE], colunas[IDX_URL]);
}
