#include "gestores/gestor_voos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_voos.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct gestor_voos
{
    GHashTable *tabela;
};

gestor_voos_t *gestor_voos_criar(void)
{
    gestor_voos_t *g = malloc(sizeof(gestor_voos_t));
    g->tabela = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)voo_destruir);
    return g;
}

void gestor_voos_destruir(gestor_voos_t *gestor)
{
    if (!gestor)
        return;
    g_hash_table_destroy(gestor->tabela);
    free(gestor);
}

void gestor_voos_adicionar(gestor_voos_t *gestor, voo_t *voo)
{
    if (!gestor || !voo)
        return;
    const char *id = voo_obter_id(voo);
    if (!id)
        return;
    g_hash_table_insert(gestor->tabela, g_strdup(id), voo);
}

voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id)
{
    if (!gestor || !flight_id)
        return NULL;
    return g_hash_table_lookup(gestor->tabela, flight_id);
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