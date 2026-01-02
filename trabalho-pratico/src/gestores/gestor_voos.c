#include "gestores/gestor_voos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_voos.h"
#include "entidades/voos.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * OTIMIZAÇÕES IMPLEMENTADAS:
 * 1. Chaves dos índices por_origin e por_destination são ponteiros partilhados
 *    com os objetos voo_t (NÃO duplicadas com g_strdup)
 * 2. Removido array 'atrasados' - Query5 filtra on-demand
 * Economia estimada: ~104 MB
 */

struct gestor_voos
{
    GHashTable *tabela;
    GHashTable *por_origin;      // ✅ Chaves partilhadas (não duplicadas)
    GHashTable *por_destination; // ✅ Chaves partilhadas (não duplicadas)
    // ✅ REMOVIDO: GPtrArray *atrasados (economiza ~8MB)
};

gestor_voos_t *gestor_voos_criar(void)
{
    gestor_voos_t *g = malloc(sizeof(gestor_voos_t));

    g->tabela = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)voo_destruir);

    // ✅ NULL no key_destroy_func - chaves NÃO são libertadas aqui
    g->por_origin = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        NULL, // ⚡ Chaves partilhadas com voos
        (GDestroyNotify)g_ptr_array_unref);

    g->por_destination = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        NULL, // ⚡ Chaves partilhadas com voos
        (GDestroyNotify)g_ptr_array_unref);

    return g;
}

void gestor_voos_destruir(gestor_voos_t *gestor)
{
    if (!gestor)
        return;

    // ✅ Destruir índices ANTES da tabela principal
    g_hash_table_destroy(gestor->por_origin);
    g_hash_table_destroy(gestor->por_destination);
    g_hash_table_destroy(gestor->tabela); // Liberta voos (e suas strings)

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

    voo_t *existente = g_hash_table_lookup(gestor->tabela, id);
    if (existente)
    {
        // Já existe - destruir o novo e não adicionar
        voo_destruir(voo);
        return;
    }

    // Adicionar à tabela principal
    g_hash_table_insert(gestor->tabela, g_strdup(id), voo);

    // ⚡ OTIMIZAÇÃO: Usar ponteiro do voo, NÃO duplicar!
    GPtrArray *voos_origin = g_hash_table_lookup(gestor->por_origin, origin);
    if (!voos_origin)
    {
        voos_origin = g_ptr_array_new();
        // ⚡ (gpointer)origin em vez de g_strdup(origin)
        g_hash_table_insert(gestor->por_origin, (gpointer)origin, voos_origin);
    }
    g_ptr_array_add(voos_origin, voo);

    // ⚡ OTIMIZAÇÃO: Mesmo para destination
    GPtrArray *voos_dest = g_hash_table_lookup(gestor->por_destination, destination);
    if (!voos_dest)
    {
        voos_dest = g_ptr_array_new();
        // ⚡ (gpointer)destination em vez de g_strdup(destination)
        g_hash_table_insert(gestor->por_destination, (gpointer)destination, voos_dest);
    }
    g_ptr_array_add(voos_dest, voo);

    // ✅ REMOVIDO: Adicionar a array 'atrasados'
    // Query5 filtra voos atrasados diretamente durante iteração
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

const char *gestor_voos_obter_departure(gestor_voos_t *gestor, const char *flight_id)
{
    if (!gestor || !flight_id)
        return NULL;

    voo_t *v = gestor_voos_obter_por_id(gestor, flight_id);
    if (!v)
        return NULL;

    return voo_obter_departure(v);
}

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

void gestor_voos_para_cada_origem(
    gestor_voos_t *gestor,
    const char *origin,
    void (*func)(voo_t *voo, void *user_data),
    void *user_data)
{
    if (!gestor || !origin || !func)
        return;

    GPtrArray *voos = g_hash_table_lookup(gestor->por_origin, origin);
    if (!voos)
        return;

    for (guint i = 0; i < voos->len; i++)
    {
        voo_t *voo = g_ptr_array_index(voos, i);
        func(voo, user_data);
    }
}

void gestor_voos_para_cada_destino(
    gestor_voos_t *gestor,
    const char *destination,
    void (*func)(voo_t *voo, void *user_data),
    void *user_data)
{
    if (!gestor || !destination || !func)
        return;

    GPtrArray *voos = g_hash_table_lookup(gestor->por_destination, destination);
    if (!voos)
        return;

    for (guint i = 0; i < voos->len; i++)
    {
        voo_t *voo = g_ptr_array_index(voos, i);
        func(voo, user_data);
    }
}