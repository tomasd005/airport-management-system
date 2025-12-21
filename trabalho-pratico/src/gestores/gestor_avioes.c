#include "gestores/gestor_avioes.h"
#include "parsers/parser.h"
#include "validacoes/validacao_avioes.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct gestor_avioes
{
    GHashTable *tabela;
};

gestor_avioes_t *gestor_avioes_criar(void)
{
    gestor_avioes_t *g = malloc(sizeof(gestor_avioes_t));
    g->tabela = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)aviao_destruir);
    return g;
}

void gestor_avioes_destruir(gestor_avioes_t *gestor)
{
    if (!gestor)
        return;
    g_hash_table_destroy(gestor->tabela);
    free(gestor);
}

void gestor_avioes_adicionar(gestor_avioes_t *gestor, aviao_t *aviao)
{
    if (!gestor || !aviao)
        return;
    const char *id = aviao_obter_identificador(aviao);
    if (!id)
        return;
    g_hash_table_insert(gestor->tabela, g_strdup(id), aviao);
}

aviao_t *gestor_avioes_obter_por_id(gestor_avioes_t *gestor, const char *identificador)
{
    if (!gestor || !identificador)
        return NULL;
    return g_hash_table_lookup(gestor->tabela, identificador);
}

GHashTable *gestor_avioes_obter_tabela(gestor_avioes_t *gestor)
{
    return gestor ? gestor->tabela : NULL;
}

unsigned gestor_avioes_contar(const gestor_avioes_t *gestor)
{
    return gestor && gestor->tabela ? g_hash_table_size(gestor->tabela) : 0;
}

void gestor_avioes_para_cada(gestor_avioes_t *gestor, void (*func)(const char *, aviao_t *, void *), void *user_data)
{
    if (!gestor || !func)
        return;
    g_hash_table_foreach(gestor->tabela, (GHFunc)func, user_data);
}

// Callback interno para o parser
static gboolean adiciona_aviao_callback(void *contexto, void *objeto)
{
    gestor_avioes_t *gestor = (gestor_avioes_t *)contexto;
    aviao_t *aviao = (aviao_t *)objeto;

    if (!gestor || !aviao)
        return FALSE;

    gestor_avioes_adicionar(gestor, aviao);
    return TRUE;
}

void gestor_avioes_carregar(gestor_avioes_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    parser_carrega(
        gestor,
        ficheiro_csv,
        adiciona_aviao_callback,
        (LinhaParaObjeto)valida_aviao,
        (DestroiObjeto)aviao_destruir);
}