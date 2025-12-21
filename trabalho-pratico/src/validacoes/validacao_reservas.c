#include "validacao_reservas.h"
#include "validacao_comum.h"
#include "../../include/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define IDX_RES_ID 0
#define IDX_FLIGHT_IDS 1
#define IDX_DOC 2
#define IDX_SEAT 3
#define IDX_PRICE 4
#define IDX_EXTRA_BAG 5
#define IDX_PRIORITY 6
#define IDX_QR 7

static gboolean valida_reservation_id(const char *res_id)
{
    if (!res_id || strlen(res_id) != 10)
        return FALSE;

    if (res_id[0] != 'R')
        return FALSE;

    for (int i = 1; i < 10; i++)
        if (!isdigit((unsigned char)res_id[i]))
            return FALSE;

    return TRUE;
}

static gboolean valida_document_number(const char *doc)
{
    if (!doc || strlen(doc) != 9)
        return FALSE;

    for (int i = 0; i < 9; i++)
        if (!isdigit((unsigned char)doc[i]))
            return FALSE;

    return TRUE;
}

static char **processar_flight_ids(const char *flight_str, size_t *out_count)
{
    if (!flight_str || !out_count)
        return NULL;

    *out_count = 0;

    char *str = g_strdup(flight_str);

    char *clean = g_malloc(strlen(str) + 1);
    char *p = str;
    char *q = clean;

    while (*p)
    {
        if (*p != '[' && *p != ']' && *p != '\'' && *p != '"' && *p != ' ')
            *q++ = *p;
        p++;
    }
    *q = '\0';

    g_free(str);

    if (clean[0] == '\0')
    {
        g_free(clean);
        return NULL;
    }

    for (p = clean; *p; p++)
        if (*p == ',')
            *p = ';';

    char **ids = g_strsplit(clean, ";", -1);
    g_free(clean);

    if (!ids)
        return NULL;

    size_t count = 0;
    for (size_t i = 0; ids[i]; i++)
    {
        utils_trim(ids[i]);
        if (ids[i][0] != '\0' && validacao_flight_id(ids[i]))
            count++;
        else
        {
            g_strfreev(ids);
            return NULL;
        }
    }

    *out_count = count;
    return ids;
}

reserva_t *valida_reserva_from_csv(char **colunas)
{
    if (!colunas)
        return NULL;

    for (int i = 0; i < 8; i++)
        if (!colunas[i])
            return NULL;

    for (int i = 0; i < 8; i++)
    {
        utils_remove_aspas(colunas[i]);
        utils_trim(colunas[i]);
    }

    if (!valida_reservation_id(colunas[IDX_RES_ID]))
        return NULL;

    if (!valida_document_number(colunas[IDX_DOC]))
        return NULL;

    size_t num_flights = 0;
    char **flight_ids = processar_flight_ids(colunas[IDX_FLIGHT_IDS], &num_flights);

    if (!flight_ids || num_flights == 0)
    {
        if (flight_ids)
            g_strfreev(flight_ids);
        return NULL;
    }

    if (!colunas[IDX_SEAT] || colunas[IDX_SEAT][0] == '\0')
    {
        g_strfreev(flight_ids);
        return NULL;
    }

    char *end;
    double price = strtod(colunas[IDX_PRICE], &end);
    if (*end != '\0' || price < 0.0)
    {
        g_strfreev(flight_ids);
        return NULL;
    }

    gboolean extra_bag = (strcmp(colunas[IDX_EXTRA_BAG], "true") == 0);
    gboolean priority = (strcmp(colunas[IDX_PRIORITY], "true") == 0);

    if (!colunas[IDX_QR] || colunas[IDX_QR][0] == '\0')
    {
        g_strfreev(flight_ids);
        return NULL;
    }

    reserva_t *r = reserva_criar(
        colunas[IDX_RES_ID],
        (const char **)flight_ids,
        num_flights,
        colunas[IDX_DOC],
        colunas[IDX_SEAT],
        price,
        extra_bag,
        priority,
        colunas[IDX_QR]);

    g_strfreev(flight_ids);
    return r;
}

GPtrArray *validar_reserva(const reserva_t *r,
                           gestor_voos_t *gestor_voos,
                           gestor_passageiros_t *gestor_passageiros)
{
    (void)gestor_voos;
    (void)gestor_passageiros;

    const char *mode = getenv("VALIDATION_MODE");

    if (mode && strcmp(mode, "OFF") == 0)
        return NULL;
    if (!r)
        return NULL;

    GPtrArray *erros = g_ptr_array_new_with_free_func(g_free);

    if (erros->len == 0)
    {
        g_ptr_array_free(erros, TRUE);
        return NULL;
    }

    if (mode && strcmp(mode, "STRICT") == 0)
        return erros;

    g_ptr_array_free(erros, TRUE);
    return NULL;
}

void validar_reserva_imprimir_erros(GPtrArray *erros)
{
    if (!erros)
        return;

    for (guint i = 0; i < erros->len; i++)
    {
        const char *msg = g_ptr_array_index(erros, i);
        if (msg)
            g_printerr("[ERRO RESERVA] %s\n", msg);
    }

    g_ptr_array_free(erros, TRUE);
}