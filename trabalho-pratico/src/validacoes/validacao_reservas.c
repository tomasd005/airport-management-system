#include "validacao_reservas.h"
#include "validacao_comum.h"
#include "../../include/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

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
    LOG_DEBUG("valida_reservation_id('%s')", res_id ? res_id : "NULL");

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
    LOG_DEBUG("valida_document_number('%s')", doc ? doc : "NULL");

    if (!doc || strlen(doc) != 9)
        return FALSE;
    for (int i = 0; i < 9; i++)
        if (!isdigit((unsigned char)doc[i]))
            return FALSE;
    return TRUE;
}

static char **processar_flight_ids(const char *flight_str, size_t *out_count)
{
    LOG_DEBUG("processar_flight_ids('%s')", flight_str ? flight_str : "NULL");

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
            LOG_DEBUG("flight_id inválido: '%s'", ids[i]);
            return NULL;
        }
    }

    *out_count = count;
    LOG_DEBUG("processar_flight_ids resultou em %zu ids válidos", count);
    return ids;
}

reserva_t *valida_reserva_from_csv(char **colunas)
{
    LOG_DEBUG("Entrou em valida_reserva_from_csv");

    if (!colunas)
        return NULL;

    for (int i = 0; i < 8; i++)
        if (!colunas[i])
            return NULL;

    for (int i = 0; i < 8; i++)
    {
        utils_remove_aspas(colunas[i]);
        utils_trim(colunas[i]);
        LOG_DEBUG("Coluna %d após limpeza: '%s'", i, colunas[i]);
    }

    if (!valida_reservation_id(colunas[IDX_RES_ID]))
        return NULL;

    if (!valida_document_number(colunas[IDX_DOC]))
        return NULL;

    size_t len = strlen(colunas[IDX_FLIGHT_IDS]);
    if (len < 2 || colunas[IDX_FLIGHT_IDS][0] != '[' || colunas[IDX_FLIGHT_IDS][len - 1] != ']')
        return NULL;

    size_t num_flights = 0;
    char **flight_ids = processar_flight_ids(colunas[IDX_FLIGHT_IDS], &num_flights);
    if (!flight_ids)
        return NULL;

    if (num_flights < 1 || num_flights > 2)
    {
        g_strfreev(flight_ids);
        LOG_DEBUG("Número de voos inválido: %zu", num_flights);
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
        LOG_DEBUG("Preço inválido: '%s'", colunas[IDX_PRICE]);
        return NULL;
    }

    if (strcmp(colunas[IDX_EXTRA_BAG], "true") != 0 && strcmp(colunas[IDX_EXTRA_BAG], "false") != 0)
    {
        g_strfreev(flight_ids);
        LOG_DEBUG("Extra_bag inválido: '%s'", colunas[IDX_EXTRA_BAG]);
        return NULL;
    }

    if (strcmp(colunas[IDX_PRIORITY], "true") != 0 && strcmp(colunas[IDX_PRIORITY], "false") != 0)
    {
        g_strfreev(flight_ids);
        LOG_DEBUG("Priority inválido: '%s'", colunas[IDX_PRIORITY]);
        return NULL;
    }

    gboolean extra_bag = (strcmp(colunas[IDX_EXTRA_BAG], "true") == 0);
    gboolean priority = (strcmp(colunas[IDX_PRIORITY], "true") == 0);

    if (!colunas[IDX_QR] || colunas[IDX_QR][0] == '\0')
    {
        g_strfreev(flight_ids);
        return NULL;
    }

    LOG_DEBUG("Validação concluída com sucesso, criando reserva_t");
    reserva_t *r = reserva_criar(colunas[IDX_RES_ID],
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

/* Validação lógica conforme enunciado */
GPtrArray *validar_reserva(const reserva_t *r,
                           gestor_voos_t *gestor_voos,
                           gestor_passageiros_t *gestor_passageiros)
{
    if (!r)
        return NULL;

    GPtrArray *erros = g_ptr_array_new_with_free_func(g_free);

    const char **flight_ids = reserva_obter_flight_ids(r);
    size_t num_voos = reserva_obter_num_voos(r);
    const char *doc = reserva_obter_document_number(r);

    // Validação: flight_ids devem corresponder a voos existentes
    if (gestor_voos)
    {
        for (size_t i = 0; i < num_voos; i++)
        {
            if (!gestor_voos_obter_por_id(gestor_voos, flight_ids[i]))
            {
                char *msg = g_strdup_printf("Flight ID '%s' não existe", flight_ids[i]);
                g_ptr_array_add(erros, msg);
            }
        }
    }

    // Validação: document_number deve corresponder a passageiro existente
    if (gestor_passageiros && doc)
    {
        if (!gestor_passageiros_obter_por_documento(gestor_passageiros, doc))
        {
            char *msg = g_strdup_printf("Document number '%s' não existe", doc);
            g_ptr_array_add(erros, msg);
        }
    }

    // Validação: se há dois voos, destination do primeiro = origin do segundo
    if (num_voos == 2 && gestor_voos)
    {
        voo_t *voo1 = gestor_voos_obter_por_id(gestor_voos, flight_ids[0]);
        voo_t *voo2 = gestor_voos_obter_por_id(gestor_voos, flight_ids[1]);

        if (voo1 && voo2)
        {
            const char *dest1 = voo_obter_destination(voo1);
            const char *orig2 = voo_obter_origin(voo2);

            if (!dest1 || !orig2 || strcmp(dest1, orig2) != 0)
            {
                char *msg = g_strdup("Destination do primeiro voo deve ser igual ao origin do segundo");
                g_ptr_array_add(erros, msg);
            }
        }
    }

    // Se não há erros, retorna NULL (sucesso)
    if (erros->len == 0)
    {
        g_ptr_array_free(erros, TRUE);
        return NULL;
    }

    return erros;
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
