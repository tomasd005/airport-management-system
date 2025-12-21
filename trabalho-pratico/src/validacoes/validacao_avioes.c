#include "validacao_avioes.h"
#include "validacao_comum.h"
#include "../../include/utils.h"
#include <string.h>

#define IDX_ID 0
#define IDX_FAB 1
#define IDX_MOD 2
#define IDX_ANO 3
#define IDX_CAP 4
#define IDX_ALC 5

aviao_t *valida_aviao(char **colunas)
{
    if (!colunas)
        return NULL;

    for (int i = 0; i <= IDX_ALC; i++)
    {
        if (colunas[i])
            utils_remove_aspas_somente(colunas[i]);
    }

    if (colunas[IDX_ANO] && *colunas[IDX_ANO])
    {
        if (contem_espacos(colunas[IDX_ANO]))
            return NULL;
    }

    for (int i = 0; i <= IDX_ALC; i++)
    {
        if (colunas[i])
            utils_trim(colunas[i]);
    }

    if (!colunas[IDX_ID] || !*colunas[IDX_ID])
        return NULL;
    if (!colunas[IDX_FAB] || !*colunas[IDX_FAB])
        return NULL;

    int cap = 0, alc = 0;
    if (!colunas[IDX_CAP] || !validacao_inteiro_positivo(colunas[IDX_CAP], &cap))
        return NULL;
    if (!colunas[IDX_ALC] || !validacao_inteiro_positivo(colunas[IDX_ALC], &alc))
        return NULL;

    int ano = 0;
    if (colunas[IDX_ANO] && *colunas[IDX_ANO])
    {
        if (!validacao_ano(colunas[IDX_ANO], &ano))
            return NULL;
    }

    const char *modelo = (colunas[IDX_MOD] && *colunas[IDX_MOD]) ? colunas[IDX_MOD] : "";

    return aviao_criar(colunas[IDX_ID], colunas[IDX_FAB], modelo, ano, cap, alc);
}
