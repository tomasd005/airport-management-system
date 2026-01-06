#include "gestores/gestor_aeroportos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_aeroportos.h"
#include "entidades/aeroportos.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct gestor_aeroportos
{
    GHashTable *aeroportos;
};

gestor_aeroportos_t *gestor_aeroportos_criar(void)
{
    gestor_aeroportos_t *g = malloc(sizeof(gestor_aeroportos_t));
    g->aeroportos = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)aeroporto_destruir);
    return g;
}

void gestor_aeroportos_destruir(gestor_aeroportos_t *gestor)
{
    if (!gestor)
        return;
    g_hash_table_destroy(gestor->aeroportos);
    free(gestor);
}

void gestor_aeroportos_adicionar(gestor_aeroportos_t *gestor, aeroporto_t *aeroporto)
{
    if (!gestor || !aeroporto)
        return;

    const char *id = aeroporto_obter_codigo(aeroporto);
    if (!id)
        return;

    if (g_hash_table_lookup(gestor->aeroportos, id))
    {
        aeroporto_destruir(aeroporto);
        return;
    }

    g_hash_table_insert(gestor->aeroportos, g_strdup(id), aeroporto);
}

aeroporto_t *gestor_aeroportos_obter_por_id(gestor_aeroportos_t *gestor, const char *id)
{
    if (!gestor || !id)
        return NULL;
    return g_hash_table_lookup(gestor->aeroportos, id);
}

aeroporto_t *gestor_aeroportos_obter_por_codigo(gestor_aeroportos_t *gestor, const char *codigo)
{
    if (!gestor || !codigo)
        return NULL;
    return g_hash_table_lookup(gestor->aeroportos, codigo);
}

unsigned gestor_aeroportos_contar(const gestor_aeroportos_t *gestor)
{
    return gestor && gestor->aeroportos ? g_hash_table_size(gestor->aeroportos) : 0;
}

void gestor_aeroportos_para_cada(gestor_aeroportos_t *gestor, void (*callback)(aeroporto_t *, void *), void *user_data)
{
    if (!gestor || !callback)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->aeroportos);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        callback((aeroporto_t *)value, user_data);
    }
}

static gboolean _adiciona_aeroporto_callback(void *contexto, void *objeto)
{
    gestor_aeroportos_t *gestor = (gestor_aeroportos_t *)contexto;
    gestor_aeroportos_adicionar(gestor, (aeroporto_t *)objeto);
    return TRUE;
}

void gestor_aeroportos_carregar(gestor_aeroportos_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;
    parser_carrega(gestor, ficheiro_csv, _adiciona_aeroporto_callback, (LinhaParaObjeto)valida_aeroporto, (DestroiObjeto)aeroporto_destruir);
}