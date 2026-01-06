#include "gestores/gestor_avioes.h"
#include "parsers/parser.h"
#include "validacoes/validacao_avioes.h"
#include "entidades/avioes.h"
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
    g->tabela = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)aviao_destruir);
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

    if (g_hash_table_lookup(gestor->tabela, id))
    {
        aviao_destruir(aviao);
        return;
    }

    g_hash_table_insert(gestor->tabela, g_strdup(id), aviao);
}

aviao_t *gestor_avioes_obter_por_id(gestor_avioes_t *gestor, const char *id)
{
    if (!gestor || !id)
        return NULL;
    return g_hash_table_lookup(gestor->tabela, id);
}

unsigned gestor_avioes_contar(const gestor_avioes_t *gestor)
{
    return gestor && gestor->tabela ? g_hash_table_size(gestor->tabela) : 0;
}

void gestor_avioes_para_cada(gestor_avioes_t *gestor, void (*func)(aviao_t *, void *), void *user_data)
{
    if (!gestor || !func)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        func((aviao_t *)value, user_data);
    }
}

static gboolean _adiciona_aviao_callback(void *contexto, void *objeto)
{
    gestor_avioes_t *gestor = (gestor_avioes_t *)contexto;
    gestor_avioes_adicionar(gestor, (aviao_t *)objeto);
    return TRUE;
}

void gestor_avioes_carregar(gestor_avioes_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;
    parser_carrega(gestor, ficheiro_csv, _adiciona_aviao_callback, (LinhaParaObjeto)valida_aviao, (DestroiObjeto)aviao_destruir);
}