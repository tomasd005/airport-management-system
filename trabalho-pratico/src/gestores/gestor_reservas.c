#include "gestores/gestor_reservas.h"
#include "gestores/gestor_voos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_reservas.h"
#include "entidades/reservas.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

struct gestor_reservas
{
    GHashTable *tabela;
    GHashTable *por_passageiro;
    GHashTable *por_voo;
};

gestor_reservas_t *gestor_reservas_criar(void)
{
    gestor_reservas_t *g = malloc(sizeof(gestor_reservas_t));
    g->tabela = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)reserva_destruir);
    g->por_passageiro = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_ptr_array_unref);
    g->por_voo = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    return g;
}

void gestor_reservas_destruir(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;

    g_hash_table_destroy(gestor->por_voo);
    g_hash_table_destroy(gestor->por_passageiro);
    g_hash_table_destroy(gestor->tabela);
    free(gestor);
}

void gestor_reservas_adicionar(gestor_reservas_t *gestor, reserva_t *reserva)
{
    if (!gestor || !reserva)
        return;

    const char *id = reserva_obter_id(reserva);
    if (!id || g_hash_table_contains(gestor->tabela, id))
    {
        reserva_destruir(reserva);
        return;
    }

    g_hash_table_insert(gestor->tabela, (gpointer)id, reserva);

    const char *doc = reserva_obter_document_number(reserva);
    if (doc)
    {
        GPtrArray *lista = g_hash_table_lookup(gestor->por_passageiro, doc);
        if (!lista)
        {
            lista = g_ptr_array_new();
            g_hash_table_insert(gestor->por_passageiro, (gpointer)doc, lista);
        }
        g_ptr_array_add(lista, reserva);
    }

    const char **flight_ids = reserva_obter_flight_ids(reserva);
    size_t num_voos = reserva_obter_num_voos(reserva);

    for (size_t i = 0; i < num_voos; i++)
    {
        if (flight_ids[i] && flight_ids[i][0])
        {
            int *count = g_hash_table_lookup(gestor->por_voo, flight_ids[i]);
            if (count)
                (*count)++;
            else
            {
                int *novo = g_new(int, 1);
                *novo = 1;
                g_hash_table_insert(gestor->por_voo, g_strdup(flight_ids[i]), novo);
            }
        }
    }
}

reserva_t *gestor_reservas_obter_por_id(gestor_reservas_t *gestor, const char *id)
{
    if (!gestor || !id)
        return NULL;
    return g_hash_table_lookup(gestor->tabela, id);
}

unsigned gestor_reservas_contar(const gestor_reservas_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->tabela) : 0;
}

unsigned int gestor_reservas_numero(gestor_reservas_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->tabela) : 0;
}

int gestor_reservas_contar_passageiros_voo(gestor_reservas_t *gestor, const char *flight_id)
{
    if (!gestor || !flight_id)
        return 0;

    int *count = g_hash_table_lookup(gestor->por_voo, flight_id);
    return count ? *count : 0;
}

GPtrArray *gestor_reservas_obter_por_passageiro(gestor_reservas_t *gestor, const char *document_number)
{
    if (!gestor || !document_number)
        return NULL;

    GPtrArray *lista_interna = g_hash_table_lookup(gestor->por_passageiro, document_number);
    if (!lista_interna || lista_interna->len == 0)
        return NULL;

    GPtrArray *resultado = g_ptr_array_new();
    for (guint i = 0; i < lista_interna->len; i++)
        g_ptr_array_add(resultado, g_ptr_array_index(lista_interna, i));

    if (resultado->len == 0)
    {
        g_ptr_array_free(resultado, TRUE);
        return NULL;
    }

    return resultado;
}

void gestor_reservas_para_cada(gestor_reservas_t *gestor, void (*func)(reserva_t *, void *), void *user_data)
{
    if (!gestor || !func)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
        func(value, user_data);
}

static gboolean _adiciona_reserva_callback(void *contexto, void *objeto)
{
    gestor_reservas_adicionar(contexto, objeto);
    return TRUE;
}

static reserva_t *_criar_reserva_de_campos(char **campos)
{
    return campos ? valida_reserva_from_csv(campos) : NULL;
}

typedef struct
{
    gestor_reservas_t *gestor_reservas;
    gestor_voos_t *gestor_voos;
    gestor_passageiros_t *gestor_passageiros;
} contexto_reservas_validacao_t;

static gboolean _adiciona_reserva_validada(void *contexto, void *objeto)
{
    contexto_reservas_validacao_t *ctx = contexto;
    reserva_t *reserva = objeto;

    GPtrArray *erros = validar_reserva(reserva, ctx->gestor_voos, ctx->gestor_passageiros);
    if (erros)
    {
        g_ptr_array_free(erros, TRUE);
        return FALSE;
    }

    gestor_reservas_adicionar(ctx->gestor_reservas, reserva);
    return TRUE;
}

void gestor_reservas_carregar(gestor_reservas_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    parser_carrega(gestor, ficheiro_csv, _adiciona_reserva_callback,
                   (LinhaParaObjeto)_criar_reserva_de_campos, (DestroiObjeto)reserva_destruir);
}

void gestor_reservas_carregar_com_validacao(
    gestor_reservas_t *gestor,
    const char *ficheiro_csv,
    gestor_voos_t *gestor_voos,
    gestor_passageiros_t *gestor_passageiros)
{
    if (!gestor || !ficheiro_csv)
        return;

    contexto_reservas_validacao_t ctx = {
        .gestor_reservas = gestor,
        .gestor_voos = gestor_voos,
        .gestor_passageiros = gestor_passageiros};

    parser_carrega(&ctx, ficheiro_csv, _adiciona_reserva_validada,
                   (LinhaParaObjeto)_criar_reserva_de_campos, (DestroiObjeto)reserva_destruir);
}

static int _is_leap(int y)
{
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static int _iso_weeks_in_year(int y)
{
    struct tm t = {0};
    t.tm_year = y - 1900;
    t.tm_mon = 0;
    t.tm_mday = 1;
    t.tm_isdst = -1;
    if (mktime(&t) == (time_t)-1)
        return 52;

    int wday = t.tm_wday == 0 ? 7 : t.tm_wday;
    return (wday == 4 || (_is_leap(y) && wday == 3)) ? 53 : 52;
}

static inline int _calcular_semana(const char *data)
{
    if (!data || strlen(data) < 10)
        return -1;

    int y, m, d;
    if (sscanf(data, "%d-%d-%d", &y, &m, &d) != 3)
        return -1;

    if (m < 1 || m > 12 || d < 1 || d > 31)
        return -1;

    struct tm t = {0};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    t.tm_hour = 12;
    t.tm_isdst = -1;

    time_t tt = mktime(&t);
    if (tt == (time_t)-1)
        return -1;

    long days = (long)(tt / 86400);
    int wday = t.tm_wday; // 0 = Domingo
    long week_start = days - wday;

    return (int)(week_start / 7);
}

void gestor_reservas_para_cada_com_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    void (*callback)(int semana, reserva_t *, void *),
    void *user_data)
{
    if (!gestor || !gestor_voos || !callback)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        reserva_t *r = value;
        const char **flight_ids = reserva_obter_flight_ids(r);
        size_t num_voos = reserva_obter_num_voos(r);

        if (num_voos > 0 && flight_ids && flight_ids[0])
        {
            voo_t *voo = gestor_voos_obter_por_id(gestor_voos, flight_ids[0]);
            if (voo)
            {
                const char *data = voo_obter_departure(voo);
                if (data)
                {
                    int semana = _calcular_semana(data);
                    if (semana > 0)
                        callback(semana, r, user_data);
                }
            }
        }
    }
}

void gestor_reservas_para_cada_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    const char *data_inicio,
    const char *data_fim,
    void (*callback)(int semana, reserva_t *, void *),
    void *user_data)
{
    if (!gestor || !gestor_voos || !data_inicio || !data_fim || !callback)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        reserva_t *r = value;
        const char **flight_ids = reserva_obter_flight_ids(r);
        size_t num_voos = reserva_obter_num_voos(r);

        if (num_voos > 0 && flight_ids && flight_ids[0])
        {
            voo_t *voo = gestor_voos_obter_por_id(gestor_voos, flight_ids[0]);
            if (!voo)
                continue;

            const char *data = voo_obter_departure(voo);
            if (!data || strlen(data) < 10)
                continue;

            if (strncmp(data, data_inicio, 10) < 0 || strncmp(data, data_fim, 10) > 0)
                continue;

            int semana = _calcular_semana(data);
            if (semana > 0)
                callback(semana, r, user_data);
        }
    }
}
