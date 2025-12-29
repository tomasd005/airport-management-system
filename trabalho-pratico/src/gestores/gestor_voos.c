#include "gestores/gestor_voos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_voos.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct gestor_voos
{
    GHashTable *tabela;          // flight_id -> voo_t*
    GHashTable *por_origin;      // origin -> GPtrArray de voo_t*
    GHashTable *por_destination; // destination -> GPtrArray de voo_t*
};

gestor_voos_t *gestor_voos_criar(void)
{
    gestor_voos_t *g = malloc(sizeof(gestor_voos_t));

    g->tabela = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)voo_destruir);

    // Hash tables para indexação rápida
    g->por_origin = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)g_ptr_array_unref);

    g->por_destination = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)g_ptr_array_unref);

    return g;
}

void gestor_voos_destruir(gestor_voos_t *gestor)
{
    if (!gestor)
        return;
    g_hash_table_destroy(gestor->tabela);
    g_hash_table_destroy(gestor->por_origin);
    g_hash_table_destroy(gestor->por_destination);
    free(gestor);
}

void gestor_voos_adicionar(gestor_voos_t *gestor, voo_t *voo)
{
    if (!gestor || !voo)
        return;

    const char *id = voo_obter_id(voo);
    const char *origin = voo_obter_origin(voo);
    const char *destination = voo_obter_destination(voo);

    if (!id || !origin || !destination)
        return;

    // Adiciona à tabela principal
    g_hash_table_insert(gestor->tabela, g_strdup(id), voo);

    // Indexa por origin
    GPtrArray *voos_origin = g_hash_table_lookup(gestor->por_origin, origin);
    if (!voos_origin)
    {
        voos_origin = g_ptr_array_new();
        g_hash_table_insert(gestor->por_origin, g_strdup(origin), voos_origin);
    }
    g_ptr_array_add(voos_origin, voo);

    // Indexa por destination
    GPtrArray *voos_dest = g_hash_table_lookup(gestor->por_destination, destination);
    if (!voos_dest)
    {
        voos_dest = g_ptr_array_new();
        g_hash_table_insert(gestor->por_destination, g_strdup(destination), voos_dest);
    }
    g_ptr_array_add(voos_dest, voo);
}

voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id)
{
    if (!gestor || !flight_id)
        return NULL;
    return g_hash_table_lookup(gestor->tabela, flight_id);
}

GPtrArray *gestor_voos_obter_por_origin(gestor_voos_t *gestor, const char *origin)
{
    if (!gestor || !origin)
        return NULL;
    return g_hash_table_lookup(gestor->por_origin, origin);
}

GPtrArray *gestor_voos_obter_por_destination(gestor_voos_t *gestor, const char *destination)
{
    if (!gestor || !destination)
        return NULL;
    return g_hash_table_lookup(gestor->por_destination, destination);
}

GHashTable *gestor_voos_obter_tabela(gestor_voos_t *gestor)
{
    return gestor ? gestor->tabela : NULL;
}

unsigned gestor_voos_contar(const gestor_voos_t *gestor)
{
    return gestor && gestor->tabela ? g_hash_table_size(gestor->tabela) : 0;
}

void gestor_voos_para_cada(gestor_voos_t *gestor, void (*func)(const char *, voo_t *, void *), void *user_data)
{
    if (!gestor || !func)
        return;
    g_hash_table_foreach(gestor->tabela, (GHFunc)func, user_data);
}

const char *gestor_voos_obter_departure(
    gestor_voos_t *gestor,
    const char *flight_id)
{
    if (!gestor || !flight_id)
        return NULL;

    voo_t *v = gestor_voos_obter_por_id(gestor, flight_id);
    if (!v)
        return NULL;

    return voo_obter_departure(v);
}

static void filtrar_atrasados(
    const char *flight_id,
    voo_t *voo,
    void *user_data)
{
    struct
    {
        void (*func)(voo_t *, void *);
        void *user_data;
    } *ctx = user_data;

    if (strcmp(voo_obter_status(voo), "Delayed") == 0)
        ctx->func(voo, ctx->user_data);
}

void gestor_voos_para_cada_atrasado(
    gestor_voos_t *gestor,
    void (*func)(voo_t *, void *),
    void *user_data)
{
    if (!gestor || !func)
        return;

    struct
    {
        void (*func)(voo_t *, void *);
        void *user_data;
    } ctx = {func, user_data};

    gestor_voos_para_cada(gestor, filtrar_atrasados, &ctx);
}

// Callback interno para o parser
static gboolean adiciona_voo_callback(void *contexto, void *objeto)
{
    gestor_voos_t *gestor = (gestor_voos_t *)contexto;
    voo_t *voo = (voo_t *)objeto;

    if (!gestor || !voo)
        return FALSE;

    gestor_voos_adicionar(gestor, voo);
    return TRUE;
}

void gestor_voos_carregar(gestor_voos_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    parser_carrega(
        gestor,
        ficheiro_csv,
        adiciona_voo_callback,
        (LinhaParaObjeto)valida_voo,
        (DestroiObjeto)voo_destruir);
}