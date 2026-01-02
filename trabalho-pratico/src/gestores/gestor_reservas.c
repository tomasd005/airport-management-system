#include "gestores/gestor_reservas.h"
#include "parsers/parser.h"
#include "validacoes/validacao_reservas.h"
#include "gestores/gestor_voos.h"
#include "gestores/gestor_passageiros.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/*
 * OTIMIZAÇÕES IMPLEMENTADAS:
 * 1. GPtrArray em vez de GArray (mais eficiente para ponteiros)
 * 2. Crescimento dinâmico (100K inicial vs 2M pré-alocados)
 * 3. Chaves do cache partilhadas com reservas (não duplicadas)
 * Economia estimada: ~60 MB
 */

struct gestor_reservas
{
    GPtrArray *reservas;              
    GHashTable *cache_por_passageiro; 
    int cache_construido;
};

gestor_reservas_t *gestor_reservas_criar(void)
{
    gestor_reservas_t *g = malloc(sizeof(gestor_reservas_t));
    if (!g)
        return NULL;

    g->reservas = g_ptr_array_new_full(100000, (GDestroyNotify)reserva_destruir);
    g->cache_por_passageiro = NULL;
    g->cache_construido = 0;

    return g;
}

void gestor_reservas_destruir(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;

    g_ptr_array_free(gestor->reservas, TRUE);

    if (gestor->cache_por_passageiro)
        g_hash_table_destroy(gestor->cache_por_passageiro);

    free(gestor);
}

void gestor_reservas_adicionar(gestor_reservas_t *gestor, reserva_t *r)
{
    if (!gestor || !r)
        return;

    g_ptr_array_add(gestor->reservas, r);
}

// ⚡ OTIMIZAÇÃO: Chaves partilhadas com reservas
static void construir_cache_passageiro(gestor_reservas_t *gestor)
{
    if (gestor->cache_construido)
        return;

    gestor->cache_por_passageiro = g_hash_table_new_full(
        g_str_hash, g_str_equal,
        NULL, // ⚡ NÃO liberar chaves!
        (GDestroyNotify)g_ptr_array_unref);

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_ptr_array_index(gestor->reservas, i);
        const char *doc = reserva_obter_document_number(r);

        if (!doc)
            continue;

        GPtrArray *arr = g_hash_table_lookup(gestor->cache_por_passageiro, doc);
        if (!arr)
        {
            arr = g_ptr_array_new();
            // ⚡ (gpointer)doc em vez de g_strdup(doc)
            g_hash_table_insert(gestor->cache_por_passageiro, (gpointer)doc, arr);
        }
        g_ptr_array_add(arr, r);
    }

    gestor->cache_construido = 1;
}

int gestor_reservas_contar_passageiros_voo(
    gestor_reservas_t *gestor,
    const char *flight_id)
{
    if (!gestor || !flight_id)
        return 0;

    int count = 0;

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_ptr_array_index(gestor->reservas, i);

        const char **flight_ids = reserva_obter_flight_ids(r);
        size_t num_voos = reserva_obter_num_voos(r);

        for (size_t j = 0; j < num_voos; j++)
        {
            if (flight_ids[j] && strcmp(flight_ids[j], flight_id) == 0)
            {
                count++;
                break;
            }
        }
    }

    return count;
}

GPtrArray *gestor_reservas_obter_por_passageiro(
    gestor_reservas_t *gestor,
    const char *document_number)
{
    if (!gestor || !document_number)
        return NULL;

    if (!gestor->cache_construido)
        construir_cache_passageiro(gestor);

    return g_hash_table_lookup(gestor->cache_por_passageiro, document_number);
}

void gestor_reservas_para_cada(
    gestor_reservas_t *gestor,
    void (*callback)(const reserva_t *, void *),
    void *user_data)
{
    if (!gestor || !callback)
        return;

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_ptr_array_index(gestor->reservas, i);
        callback(r, user_data);
    }
}

static int calcular_semana(const char *data)
{
    if (!data || strlen(data) < 10)
        return -1;

    int y, m, d;
    if (sscanf(data, "%d-%d-%d", &y, &m, &d) != 3)
        return -1;

    struct tm t = {0};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    t.tm_hour = 12;
    t.tm_isdst = -1;
    mktime(&t);

    int dia_domingo = t.tm_yday - t.tm_wday;
    if (dia_domingo < 0)
        dia_domingo = 0;

    return (y * 1000) + (dia_domingo / 7);
}

void gestor_reservas_para_cada_com_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    void (*callback)(int, const reserva_t *, void *),
    void *user_data)
{
    if (!gestor || !gestor_voos || !callback)
        return;

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_ptr_array_index(gestor->reservas, i);

        const char **flight_ids = reserva_obter_flight_ids(r);
        if (!flight_ids || !flight_ids[0])
            continue;

        const char *dep = gestor_voos_obter_departure(gestor_voos, flight_ids[0]);
        if (!dep || strlen(dep) < 10)
            continue;

        char data[11];
        strncpy(data, dep, 10);
        data[10] = '\0';

        int semana = calcular_semana(data);
        if (semana >= 0)
            callback(semana, r, user_data);
    }
}

void gestor_reservas_para_cada_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    const char *data_inicio,
    const char *data_fim,
    void (*callback)(int, const reserva_t *, void *),
    void *user_data)
{
    if (!gestor || !gestor_voos || !callback)
        return;

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_ptr_array_index(gestor->reservas, i);

        const char **flight_ids = reserva_obter_flight_ids(r);
        if (!flight_ids || !flight_ids[0])
            continue;

        const char *dep = gestor_voos_obter_departure(gestor_voos, flight_ids[0]);
        if (!dep || strlen(dep) < 10)
            continue;

        char data[11];
        strncpy(data, dep, 10);
        data[10] = '\0';

        if (data_inicio && strcmp(data, data_inicio) < 0)
            continue;
        if (data_fim && strcmp(data, data_fim) > 0)
            continue;

        int semana = calcular_semana(data);
        if (semana >= 0)
            callback(semana, r, user_data);
    }
}

typedef struct
{
    gestor_reservas_t *gestor_reservas;
    gestor_voos_t *gestor_voos;
    gestor_passageiros_t *gestor_passageiros;
} ContextoCarregamento;

static gboolean adiciona_reserva_simples(void *contexto, void *objeto)
{
    gestor_reservas_t *gestor = contexto;
    reserva_t *reserva = objeto;

    if (!gestor || !reserva)
        return FALSE;

    gestor_reservas_adicionar(gestor, reserva);
    return TRUE;
}

void gestor_reservas_carregar(
    gestor_reservas_t *gestor,
    const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    parser_carrega(
        gestor, ficheiro_csv,
        adiciona_reserva_simples,
        (LinhaParaObjeto)valida_reserva_from_csv,
        (DestroiObjeto)reserva_destruir);
}

static gboolean adiciona_reserva_callback(void *contexto, void *objeto)
{
    ContextoCarregamento *ctx = contexto;
    reserva_t *reserva = objeto;

    if (!ctx || !ctx->gestor_reservas || !reserva)
        return FALSE;

    GPtrArray *erros = validar_reserva(
        reserva, ctx->gestor_voos, ctx->gestor_passageiros);

    if (erros)
    {
        g_ptr_array_free(erros, TRUE);
        return FALSE;
    }

    gestor_reservas_adicionar(ctx->gestor_reservas, reserva);
    return TRUE;
}

void gestor_reservas_carregar_com_validacao(
    gestor_reservas_t *gestor,
    const char *ficheiro_csv,
    gestor_voos_t *gestor_voos,
    gestor_passageiros_t *gestor_passageiros)
{
    if (!gestor || !ficheiro_csv)
        return;

    ContextoCarregamento ctx = {
        .gestor_reservas = gestor,
        .gestor_voos = gestor_voos,
        .gestor_passageiros = gestor_passageiros};

    parser_carrega(
        &ctx, ficheiro_csv,
        adiciona_reserva_callback,
        (LinhaParaObjeto)valida_reserva_from_csv,
        (DestroiObjeto)reserva_destruir);
}

unsigned int gestor_reservas_numero(gestor_reservas_t *gestor)
{
    return gestor ? gestor->reservas->len : 0;
}

reserva_t *gestor_reservas_obter_por_id(
    gestor_reservas_t *gestor,
    const char *reservation_id)
{
    if (!gestor || !reservation_id)
        return NULL;

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_ptr_array_index(gestor->reservas, i);
        if (strcmp(reserva_obter_id(r), reservation_id) == 0)
            return r;
    }
    return NULL;
}