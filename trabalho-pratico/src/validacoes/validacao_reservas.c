#include "validacao_reservas.h"
#include "validacao_comum.h"
#include "parsers/parser.h"
#include "../../include/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/** Índices das colunas do CSV de reservas */
#define IDX_RES_ID 0
#define IDX_FLIGHT_IDS 1
#define IDX_DOC 2
#define IDX_SEAT 3
#define IDX_PRICE 4
#define IDX_EXTRA_BAG 5
#define IDX_PRIORITY 6
#define IDX_QR 7

/**
 * @brief Valida o ID da reserva.
 *
 * Deve começar com 'R' seguido de 9 dígitos.
 *
 * @param res_id ID da reserva
 * @return TRUE se válido, FALSE caso contrário
 */
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

/**
 * @brief Valida o número de documento do passageiro.
 *
 * Deve conter exatamente 9 dígitos.
 *
 * @param doc Número de documento
 * @return TRUE se válido, FALSE caso contrário
 */
static gboolean valida_document_number(const char *doc)
{
    if (!doc || strlen(doc) != 9)
        return FALSE;
    for (int i = 0; i < 9; i++)
        if (!isdigit((unsigned char)doc[i]))
            return FALSE;
    return TRUE;
}

static gboolean valida_flight_id_rapida(const char *flight_id)
{
    if (!flight_id)
        return FALSE;

    if (!isupper((unsigned char)flight_id[0]) || !isupper((unsigned char)flight_id[1]))
        return FALSE;

    int len = 2;
    for (const char *p = flight_id + 2; *p; p++) {
        if (!isdigit((unsigned char)*p))
            return FALSE;
        len++;
        if (len > 9)
            return FALSE;
    }

    return (len >= 7 && len <= 9);
}

static size_t processar_flight_ids_inplace(char *flight_str, const char **out_ids)
{
    if (!flight_str || !out_ids)
        return 0;

    size_t len = strlen(flight_str);
    if (len < 2 || flight_str[0] != '[' || flight_str[len - 1] != ']')
        return 0;

    flight_str[len - 1] = '\0';
    char *p = flight_str + 1;
    size_t count = 0;

    while (*p) {
        while (*p && (isspace((unsigned char)*p) || *p == '\'' || *p == '"'))
            p++;
        if (!*p)
            break;

        char *start = p;
        while (*p && *p != ',' && *p != ';')
            p++;
        char *end = p;

        while (end > start && isspace((unsigned char)end[-1]))
            end--;
        while (end > start && (end[-1] == '\'' || end[-1] == '"'))
            end--;
        *end = '\0';

        if (!*start || !valida_flight_id_rapida(start))
            return 0;

        if (count >= 2)
            return 0;
        out_ids[count++] = start;

        if (*p)
            p++;
    }

    return count;
}

static size_t processar_flight_ids_inplace_fast(char *flight_str, const char **out_ids)
{
    if (!flight_str || !out_ids)
        return 0;

    while (*flight_str && isspace((unsigned char)*flight_str))
        flight_str++;

    size_t len = strlen(flight_str);
    while (len > 0 && isspace((unsigned char)flight_str[len - 1]))
        flight_str[--len] = '\0';

    if (len < 2 || flight_str[0] != '[' || flight_str[len - 1] != ']')
        return 0;

    flight_str[len - 1] = '\0';
    char *p = flight_str + 1;
    size_t count = 0;

    while (*p) {
        while (*p == ' ' || *p == '\'' || *p == '"')
            p++;
        if (!*p)
            break;

        if (count >= 2)
            return 0;

        char *start = p;
        while (*p && *p != ',' && *p != ';')
            p++;
        char *end = p;

        while (end > start && (end[-1] == ' ' || end[-1] == '\'' || end[-1] == '"'))
            end--;
        *end = '\0';

        if (!*start)
            return 0;

        out_ids[count++] = start;

        if (*p) {
            *p = '\0';
            p++;
        }
    }

    return count;
}

static double parse_preco_fast(const char *s)
{
    if (!s || !*s)
        return -1.0;

    double val = 0.0;
    while (*s && *s >= '0' && *s <= '9') {
        val = val * 10.0 + (double)(*s - '0');
        s++;
    }

    if (*s == '.') {
        s++;
        double base = 0.1;
        while (*s && *s >= '0' && *s <= '9') {
            val += (double)(*s - '0') * base;
            base *= 0.1;
            s++;
        }
    }

    return val;
}

static int parse_preco_valid(const char *s, double *out_val)
{
    if (!s || !*s || !out_val)
        return 0;

    double val = 0.0;
    int saw_digit = 0;
    while (*s && *s >= '0' && *s <= '9') {
        val = val * 10.0 + (double)(*s - '0');
        s++;
        saw_digit = 1;
    }

    if (!saw_digit)
        return 0;

    if (*s == '.') {
        s++;
        double base = 0.1;
        if (*s < '0' || *s > '9')
            return 0;
        while (*s && *s >= '0' && *s <= '9') {
            val += (double)(*s - '0') * base;
            base *= 0.1;
            s++;
        }
    }

    if (*s != '\0')
        return 0;

    *out_val = val;
    return 1;
}

static inline uint32_t document_key_from_str_fast(const char *doc)
{
    if (*doc == '"' || *doc == '\'')
        doc++;
    uint32_t value = 0;
    for (int i = 0; i < 9; i++) {
        value = value * 10u + (uint32_t)(doc[i] - '0');
    }
    return value;
}

static char *strip_outer_quotes(char *s)
{
    if (!s)
        return s;

    size_t len = strlen(s);
    if (len >= 2 && ((s[0] == '"' && s[len - 1] == '"') || (s[0] == '\'' && s[len - 1] == '\''))) {
        s[len - 1] = '\0';
        return s + 1;
    }

    return s;
}
/**
 * @brief Valida os campos de uma reserva e retorna informações importantes.
 *
 * @param colunas Array de strings com os campos da reserva
 * @param out_flight_ids Array para retornar os flight IDs
 * @param out_num_voos Ponteiro para número de voos
 * @param out_document_number Ponteiro para número de documento
 * @param out_preco Ponteiro para preço
 * @return TRUE se os campos são válidos, FALSE caso contrário
 */
gboolean valida_reserva_campos(char **colunas, const char **out_flight_ids, size_t *out_num_voos,
                               const char **out_document_number, uint32_t *out_document_key,
                               double *out_preco)
{
    if (!colunas || !out_flight_ids || !out_num_voos || !out_document_number || !out_document_key ||
        !out_preco)
        return FALSE;

    for (int i = 0; i < 8; i++)
        if (!colunas[i])
            return FALSE;

    if (parser_sem_erros_ativo()) {
        char *flight_ids = strip_outer_quotes(colunas[IDX_FLIGHT_IDS]);
        const char *doc = colunas[IDX_DOC];
        char *price_str = strip_outer_quotes(colunas[IDX_PRICE]);

        if (!doc || !*doc)
            return FALSE;
        *out_document_key = document_key_from_str_fast(doc);

        double price = parse_preco_fast(price_str);
        if (price < 0.0)
            return FALSE;

        size_t num_flights = processar_flight_ids_inplace_fast(flight_ids, out_flight_ids);
        if (num_flights < 1 || num_flights > 2)
            return FALSE;

        *out_num_voos = num_flights;
        *out_document_number = doc;
        *out_preco = price;
        return TRUE;
    }

    for (int i = 0; i < 8; i++) {
        utils_remove_aspas_somente(colunas[i]);
    }

    if (!valida_reservation_id(colunas[IDX_RES_ID]))
        return FALSE;

    if (!valida_document_number(colunas[IDX_DOC]))
        return FALSE;

    if (!colunas[IDX_SEAT] || colunas[IDX_SEAT][0] == '\0')
        return FALSE;

    double price = 0.0;
    if (!parse_preco_valid(colunas[IDX_PRICE], &price) || price < 0.0)
        return FALSE;

    if (strcmp(colunas[IDX_EXTRA_BAG], "true") != 0 && strcmp(colunas[IDX_EXTRA_BAG], "false") != 0)
        return FALSE;

    if (strcmp(colunas[IDX_PRIORITY], "true") != 0 && strcmp(colunas[IDX_PRIORITY], "false") != 0)
        return FALSE;

    if (!colunas[IDX_QR] || colunas[IDX_QR][0] == '\0')
        return FALSE;

    size_t num_flights = processar_flight_ids_inplace(colunas[IDX_FLIGHT_IDS], out_flight_ids);
    if (num_flights < 1 || num_flights > 2)
        return FALSE;

    if (!utils_document_number_key(colunas[IDX_DOC], out_document_key))
        return FALSE;

    *out_num_voos = num_flights;
    *out_document_number = colunas[IDX_DOC];
    *out_preco = price;
    return TRUE;
}

/**
 * @brief Valida logicamente uma reserva, retornando erros se houver.
 *
 * Regras:
 * - Flight IDs existem
 * - Passageiro existe
 * - Se dois voos, destino do primeiro = origem do segundo
 *
 * @param r Reserva a validar
 * @param gestor_voos Gestor de voos
 * @param gestor_passageiros Gestor de passageiros
 * @return GPtrArray com strings de erros ou NULL se válido
 */
