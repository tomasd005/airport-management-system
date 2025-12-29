#include "gestores/gestor_reservas.h"
#include "parsers/parser.h"
#include "validacoes/validacao_reservas.h"
#include "gestores/gestor_voos.h"
#include "gestores/gestor_passageiros.h"
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

struct gestor_reservas
{
    GArray *reservas;
    GHashTable *por_flight_id; // flight_id -> GPtrArray de reservas
    GHashTable *passageiros_por_voo; // flight_id -> GHashTable de document_numbers (set)
};

gestor_reservas_t *gestor_reservas_criar(void)
{
    gestor_reservas_t *g = malloc(sizeof(gestor_reservas_t));
    g->reservas = g_array_new(FALSE, FALSE, sizeof(reserva_t *));

    g->por_flight_id = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)g_ptr_array_unref);

    // Índice otimizado para contagem rápida de passageiros únicos por voo
    g->passageiros_por_voo = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)g_hash_table_destroy);

    return g;
}

void gestor_reservas_destruir(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;
    for (guint i = 0; i < gestor->reservas->len; i++)
        reserva_destruir(g_array_index(gestor->reservas, reserva_t *, i));
    g_array_free(gestor->reservas, TRUE);
    g_hash_table_destroy(gestor->por_flight_id);
    g_hash_table_destroy(gestor->passageiros_por_voo);
    free(gestor);
}

void gestor_reservas_adicionar(gestor_reservas_t *gestor, reserva_t *r)
{
    if (!gestor || !r)
        return;

    g_array_append_val(gestor->reservas, r);

    /* Atualizar índices por flight_id */
    const char **flight_ids = reserva_obter_flight_ids(r);
    size_t num_voos = reserva_obter_num_voos(r);
    const char *doc = reserva_obter_document_number(r);

    for (size_t i = 0; i < num_voos; i++)
    {
        const char *flight_id = flight_ids[i];
        if (!flight_id)
            continue;

        /* Índice por flight_id -> lista de reservas */
        GPtrArray *lista = g_hash_table_lookup(gestor->por_flight_id, flight_id);
        if (!lista)
        {
            lista = g_ptr_array_new();
            g_hash_table_insert(gestor->por_flight_id, g_strdup(flight_id), lista);
        }
        g_ptr_array_add(lista, r);

        /* Índice otimizado: flight_id -> set de document_numbers */
        GHashTable *passageiros_set = g_hash_table_lookup(gestor->passageiros_por_voo, flight_id);
        if (!passageiros_set)
        {
            /* criar set com free para as chaves (as chaves serão cópias dos docs) */
            passageiros_set = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
            g_hash_table_insert(gestor->passageiros_por_voo, g_strdup(flight_id), passageiros_set);
        }

        if (doc && *doc)
        {
            g_hash_table_add(passageiros_set, g_strdup(doc));
        }
    }
}

reserva_t *gestor_reservas_obter_por_id(gestor_reservas_t *gestor, const char *reservation_id)
{
    if (!gestor || !reservation_id)
        return NULL;

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_array_index(gestor->reservas, reserva_t *, i);
        if (strcmp(reserva_obter_id(r), reservation_id) == 0)
            return r;
    }
    return NULL;
}

int gestor_reservas_contar_passageiros_voo(gestor_reservas_t *gestor, const char *flight_id)
{
    if (!gestor || !flight_id)
        return 0;

    // OTIMIZAÇÃO: Usa índice pré-construído O(1) em vez de iterar todas as reservas
    GHashTable *passageiros_set = g_hash_table_lookup(gestor->passageiros_por_voo, flight_id);
    if (!passageiros_set)
        return 0;

    return g_hash_table_size(passageiros_set);
}

unsigned int gestor_reservas_numero(gestor_reservas_t *gestor)
{
    return gestor ? gestor->reservas->len : 0;
}

// Contexto para callback com validações lógicas
typedef struct
{
    gestor_reservas_t *gestor_reservas;
    gestor_voos_t *gestor_voos;
    gestor_passageiros_t *gestor_passageiros;
} ContextoCarregamento;

static gboolean adiciona_reserva_callback(void *contexto, void *objeto)
{
    ContextoCarregamento *ctx = (ContextoCarregamento *)contexto;
    reserva_t *reserva = (reserva_t *)objeto;

    if (!ctx || !ctx->gestor_reservas || !reserva)
        return FALSE;

    // Fazer validações lógicas
    GPtrArray *erros = validar_reserva(reserva, ctx->gestor_voos, ctx->gestor_passageiros);
    
    if (erros)
    {
        // Há erros de validação lógica - não adiciona a reserva
        validar_reserva_imprimir_erros(erros);
        return FALSE;
    }

    gestor_reservas_adicionar(ctx->gestor_reservas, reserva);
    return TRUE;
}

// Callback simples sem validações lógicas (para compatibilidade)
static gboolean adiciona_reserva_simples_callback(void *contexto, void *objeto)
{
    gestor_reservas_t *gestor = (gestor_reservas_t *)contexto;
    reserva_t *reserva = (reserva_t *)objeto;

    if (!gestor || !reserva)
        return FALSE;

    gestor_reservas_adicionar(gestor, reserva);
    return TRUE;
}

void gestor_reservas_carregar(gestor_reservas_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    // Carregar sem validações lógicas (para compatibilidade)
    parser_carrega(
        gestor,
        ficheiro_csv,
        adiciona_reserva_simples_callback,
        (LinhaParaObjeto)valida_reserva_from_csv,
        (DestroiObjeto)reserva_destruir);
}

// Nova função para carregar com validações lógicas
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
        .gestor_passageiros = gestor_passageiros
    };

    parser_carrega(
        &ctx,
        ficheiro_csv,
        adiciona_reserva_callback,
        (LinhaParaObjeto)valida_reserva_from_csv,
        (DestroiObjeto)reserva_destruir);
}

static int calcular_semana(const char *data)
{
    int y, m, d;
    sscanf(data, "%d-%d-%d", &y, &m, &d);

    struct tm t = {0};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    t.tm_hour = 12; // Evitar problemas com DST
    t.tm_isdst = -1;
    mktime(&t);

    // tm_wday: 0=domingo, 1=segunda, ..., 6=sábado
    // Para encontrar o domingo da semana: subtrair tm_wday dias
    // Se for domingo (tm_wday=0), fica o mesmo dia
    // Se for sábado (tm_wday=6), subtrai 6 para chegar ao domingo
    int dia_domingo = t.tm_yday - t.tm_wday;
    
    // Se o resultado for negativo, significa que estamos no início do ano
    // e o domingo está no ano anterior, mas isso não deve acontecer na prática
    // para simplificar, usamos o próprio dia se for negativo
    if (dia_domingo < 0)
        dia_domingo = t.tm_yday;

    return (y * 1000) + dia_domingo;
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
        reserva_t *r = g_array_index(gestor->reservas, reserva_t *, i);

        const char **flight_ids = reserva_obter_flight_ids(r);
        const char *dep = gestor_voos_obter_departure(
            gestor_voos, flight_ids[0]);

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
        callback(semana, r, user_data);
    }
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
        reserva_t *r = g_array_index(gestor->reservas, reserva_t *, i);
        callback(r, user_data);
    }
}

void gestor_reservas_para_cada_com_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    void (*callback)(int semana, const reserva_t *, void *),
    void *user_data)
{
    if (!gestor || !gestor_voos || !callback)
        return;

    for (guint i = 0; i < gestor->reservas->len; i++)
    {
        reserva_t *r = g_array_index(gestor->reservas, reserva_t *, i);

        const char **flight_ids = reserva_obter_flight_ids(r);
        const char *dep = gestor_voos_obter_departure(
            gestor_voos, flight_ids[0]);

        if (!dep || strlen(dep) < 10)
            continue;

        char data[11];
        strncpy(data, dep, 10);
        data[10] = '\0';

        int semana = calcular_semana(data);
        callback(semana, r, user_data);
    }
}