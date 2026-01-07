#include "validacao_voos.h"
#include "validacao_comum.h"
#include "../../include/utils.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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

static FILE *abre_ficheiro_erros(void)
{
    static FILE *fp = NULL;
    if (!fp)
    {
        fp = fopen("resultados/flights_errors.csv", "w");
        if (fp)
        {
            fprintf(fp, "\"flight id\",\"departure\",\"actual departure\",\"arrival\",\"actual arrival\",\"gate\",\"status\",\"origin\",\"destination\",\"aircraft\",\"airline\",\"url\"\n");
        }
    }
    return fp;
}

static void grava_erro(char **colunas)
{
    FILE *fp = abre_ficheiro_erros();
    if (!fp)
        return;
    for (int i = 0; i <= IDX_URL; i++)
    {
        fprintf(fp, "\"%s\"", colunas[i] ? colunas[i] : "");
        if (i < IDX_URL)
            fputc(',', fp);
    }
    fputc('\n', fp);
}

static gboolean validacao_flight_id_strict(const char *str)
{
    if (!str || contem_espacos(str))
        return FALSE;

    size_t len = strlen(str);
    if (len < 7 || len > 9)
        return FALSE;

    if (!isupper(str[0]) || !isupper(str[1]))
        return FALSE;

    for (size_t i = 2; i < len; i++)
        if (!isdigit(str[i]))
            return FALSE;

    return TRUE;
}

voo_t *valida_voo(char **colunas)
{
    if (!colunas || !colunas[IDX_ID])
        return NULL;

    for (int i = 0; i <= IDX_URL; i++)
        if (colunas[i])
            utils_remove_aspas_somente(colunas[i]);

    if (!colunas[IDX_ID] || !validacao_flight_id_strict(colunas[IDX_ID]))
    {
        grava_erro(colunas);
        return NULL;
    }

    if ((colunas[IDX_ORIG] && contem_espacos(colunas[IDX_ORIG])) ||
        (colunas[IDX_DEST] && contem_espacos(colunas[IDX_DEST])))
    {
        grava_erro(colunas);
        return NULL;
    }

    for (int i = 0; i <= IDX_URL; i++)
        if (colunas[i])
            utils_trim(colunas[i]);

    if (!*colunas[IDX_ID] || !*colunas[IDX_ORIG] || !*colunas[IDX_DEST] ||
        !*colunas[IDX_AIR] || !*colunas[IDX_STATUS] || !*colunas[IDX_DEP] || !*colunas[IDX_ARR])
    {
        grava_erro(colunas);
        return NULL;
    }

    if (strlen(colunas[IDX_ORIG]) != 3 || strlen(colunas[IDX_DEST]) != 3 ||
        !isupper(colunas[IDX_ORIG][0]) || !isupper(colunas[IDX_ORIG][1]) || !isupper(colunas[IDX_ORIG][2]) ||
        !isupper(colunas[IDX_DEST][0]) || !isupper(colunas[IDX_DEST][1]) || !isupper(colunas[IDX_DEST][2]) ||
        strcmp(colunas[IDX_ORIG], colunas[IDX_DEST]) == 0 ||
        !validacao_datetime(colunas[IDX_DEP]) || !validacao_datetime(colunas[IDX_ARR]))
    {
        grava_erro(colunas);
        return NULL;
    }

    if (comparar_datetime(colunas[IDX_ARR], colunas[IDX_DEP]) < 0)
    {
        grava_erro(colunas);
        return NULL;
    }

    if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 && !validacao_datetime(colunas[IDX_ACT_DEP]))
    {
        grava_erro(colunas);
        return NULL;
    }

    if (strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 && !validacao_datetime(colunas[IDX_ACT_ARR]))
    {
        grava_erro(colunas);
        return NULL;
    }

    if (strcmp(colunas[IDX_STATUS], "On Time") != 0 &&
        strcmp(colunas[IDX_STATUS], "Delayed") != 0 &&
        strcmp(colunas[IDX_STATUS], "Cancelled") != 0)
    {
        grava_erro(colunas);
        return NULL;
    }

    if (strcmp(colunas[IDX_STATUS], "Cancelled") == 0 &&
        (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 || strcmp(colunas[IDX_ACT_ARR], "N/A") != 0))
    {
        grava_erro(colunas);
        return NULL;
    }

    if (strcmp(colunas[IDX_STATUS], "Delayed") == 0)
    {
        if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 && comparar_datetime(colunas[IDX_ACT_DEP], colunas[IDX_DEP]) < 0)
        {
            grava_erro(colunas);
            return NULL;
        }
        if (strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 && comparar_datetime(colunas[IDX_ACT_ARR], colunas[IDX_ARR]) < 0)
        {
            grava_erro(colunas);
            return NULL;
        }
    }

    if (strcmp(colunas[IDX_ACT_DEP], "N/A") != 0 && strcmp(colunas[IDX_ACT_ARR], "N/A") != 0 &&
        comparar_datetime(colunas[IDX_ACT_ARR], colunas[IDX_ACT_DEP]) < 0)
    {
        grava_erro(colunas);
        return NULL;
    }

    return voo_criar(colunas[IDX_ID], colunas[IDX_DEP], colunas[IDX_ACT_DEP],
                     colunas[IDX_ARR], colunas[IDX_ACT_ARR], colunas[IDX_GATE],
                     colunas[IDX_STATUS], colunas[IDX_ORIG], colunas[IDX_DEST],
                     colunas[IDX_AIR], colunas[IDX_AIRLINE], colunas[IDX_URL]);
}
