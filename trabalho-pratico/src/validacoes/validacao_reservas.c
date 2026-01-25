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

/**
 * @brief Processa a string de flight IDs de uma reserva.
 *
 * Remove caracteres não relevantes e valida cada ID.
 *
 * @param flight_str String com flight IDs
 * @param out_count Ponteiro para armazenar o número de IDs válidos
 * @return Array de strings com os IDs válidos (deve ser liberado com g_strfreev)
 */
static char **processar_flight_ids(const char *flight_str, size_t *out_count)
{
    if (!flight_str || !out_count)
        return NULL;

    *out_count = 0;
    char *str = g_strdup(flight_str);
    char *clean = g_malloc(strlen(str) + 1);
    char *p = str;
    char *q = clean;

    while (*p) {
        if (*p != '[' && *p != ']' && *p != '\'' && *p != '"' && *p != ' ')
            *q++ = *p;
        p++;
    }
    *q = '\0';
    g_free(str);

    if (clean[0] == '\0') {
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
    for (size_t i = 0; ids[i]; i++) {
        utils_trim(ids[i]);
        if (ids[i][0] != '\0' && validacao_flight_id(ids[i]))
            count++;
        else {
            g_strfreev(ids);
            return NULL;
        }
    }

    *out_count = count;

    return ids;
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

        if (!*start || !validacao_flight_id(start))
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

static int document_key_from_str(const char *doc, uint32_t *out_key)
{
    if (!doc || !out_key)
        return 0;

    if (*doc == '"' || *doc == '\'')
        doc++;

    uint32_t value = 0;
    for (int i = 0; i < 9; i++) {
        char c = doc[i];
        if (c < '0' || c > '9')
            return 0;
        value = value * 10u + (uint32_t)(c - '0');
    }

    char term = doc[9];
    if (term != '\0' && term != '"' && term != '\'')
        return 0;

    *out_key = value;
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
 * @brief Valida os campos de uma reserva a partir de um CSV.
 *
 * @param colunas Array de strings com os campos da reserva
 * @return ponteiro para reserva criada se válido, NULL caso contrário
 */
reserva_t *valida_reserva_from_csv(char **colunas)
{
    if (!colunas)
        return NULL;

    for (int i = 0; i < 8; i++)
        if (!colunas[i])
            return NULL;

    for (int i = 0; i < 8; i++) {
        utils_remove_aspas(colunas[i]);
        utils_trim(colunas[i]);
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

    if (num_flights < 1 || num_flights > 2) {
        g_strfreev(flight_ids);
        return NULL;
    }

    if (!colunas[IDX_SEAT] || colunas[IDX_SEAT][0] == '\0') {
        g_strfreev(flight_ids);
        return NULL;
    }

    char *end;
    double price = strtod(colunas[IDX_PRICE], &end);
    if (*end != '\0' || price < 0.0) {
        g_strfreev(flight_ids);
        return NULL;
    }

    if (strcmp(colunas[IDX_EXTRA_BAG], "true") != 0 &&
        strcmp(colunas[IDX_EXTRA_BAG], "false") != 0) {
        g_strfreev(flight_ids);
        return NULL;
    }

    if (strcmp(colunas[IDX_PRIORITY], "true") != 0 && strcmp(colunas[IDX_PRIORITY], "false") != 0) {
        g_strfreev(flight_ids);
        return NULL;
    }

    gboolean extra_bag = (strcmp(colunas[IDX_EXTRA_BAG], "true") == 0);
    gboolean priority = (strcmp(colunas[IDX_PRIORITY], "true") == 0);

    if (!colunas[IDX_QR] || colunas[IDX_QR][0] == '\0') {
        g_strfreev(flight_ids);
        return NULL;
    }

    reserva_t *r =
        reserva_criar(colunas[IDX_RES_ID], (const char **)flight_ids, num_flights, colunas[IDX_DOC],
                      colunas[IDX_SEAT], price, extra_bag, priority, colunas[IDX_QR]);

    g_strfreev(flight_ids);
    return r;
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
        utils_remove_aspas(colunas[i]);
        utils_trim(colunas[i]);
    }

    if (!valida_reservation_id(colunas[IDX_RES_ID]))
        return FALSE;

    if (!valida_document_number(colunas[IDX_DOC]))
        return FALSE;

    if (!colunas[IDX_SEAT] || colunas[IDX_SEAT][0] == '\0')
        return FALSE;

    char *end;
    double price = strtod(colunas[IDX_PRICE], &end);
    if (*end != '\0' || price < 0.0)
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
GPtrArray *validar_reserva(const reserva_t *r, gestor_voos_t *gestor_voos,
                           gestor_passageiros_t *gestor_passageiros)
{
    if (!r)
        return NULL;

    GPtrArray *erros = g_ptr_array_new_with_free_func(g_free);

    const char **flight_ids = reserva_obter_flight_ids(r);
    size_t num_voos = reserva_obter_num_voos(r);
    const char *doc = reserva_obter_document_number(r);

    // Validação: flight_ids devem corresponder a voos existentes
    if (gestor_voos) {
        for (size_t i = 0; i < num_voos; i++) {
            if (!gestor_voos_obter_por_id(gestor_voos, flight_ids[i])) {
                char *msg = g_strdup_printf("Flight ID '%s' não existe", flight_ids[i]);
                g_ptr_array_add(erros, msg);
            }
        }
    }

    // Validação: document_number deve corresponder a passageiro existente
    if (gestor_passageiros && doc) {
        if (!gestor_passageiros_obter_por_documento(gestor_passageiros, doc)) {
            char *msg = g_strdup_printf("Document number '%s' não existe", doc);
            g_ptr_array_add(erros, msg);
        }
    }

    // Validação: se há dois voos, destination do primeiro = origin do segundo
    if (num_voos == 2 && gestor_voos) {
        voo_t *voo1 = gestor_voos_obter_por_id(gestor_voos, flight_ids[0]);
        voo_t *voo2 = gestor_voos_obter_por_id(gestor_voos, flight_ids[1]);

        if (voo1 && voo2) {
            int dest1 = voo_obter_destination_idx(voo1);
            int orig2 = voo_obter_origin_idx(voo2);

            if (dest1 < 0 || orig2 < 0 || dest1 != orig2) {
                char *msg =
                    g_strdup("Destination do primeiro voo deve ser igual ao origin do segundo");
                g_ptr_array_add(erros, msg);
            }
        }
    }

    // Se não há erros, retorna NULL (sucesso)
    if (erros->len == 0) {
        g_ptr_array_free(erros, TRUE);
        return NULL;
    }

    return erros;
}

/**
 * @brief Liberta a memória de um array de erros de validação.
 *
 * @param erros GPtrArray com erros
 */
void validar_reserva_imprimir_erros(GPtrArray *erros)
{
    if (!erros)
        return;

    g_ptr_array_free(erros, TRUE);
}
