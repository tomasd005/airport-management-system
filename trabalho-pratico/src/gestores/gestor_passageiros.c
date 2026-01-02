#include "gestores/gestor_passageiros.h"
#include "parsers/parser.h"
#include "validacoes/validacao_passageiros.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

/*
 * OTIMIZAÇÕES IMPLEMENTADAS:
 * 1. GPtrArray em vez de GArray
 * 2. Chaves partilhadas no índice por_documento
 * 3. Crescimento dinâmico (50K inicial)
 * Economia estimada: ~30 MB
 */

struct gestor_passageiros
{
    GPtrArray *passageiros;        
    GHashTable *por_documento;    
    GHashTable *por_nacionalidade; 
};

gestor_passageiros_t *gestor_passageiros_criar(void)
{
    gestor_passageiros_t *g = malloc(sizeof(gestor_passageiros_t));
    
    g->passageiros = g_ptr_array_new_full(50000, (GDestroyNotify)passageiro_destruir);

    g->por_documento = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        NULL,  // ⚡ NÃO liberar chaves
        NULL); // ⚡ NÃO liberar valores (geridos pelo array)

    g->por_nacionalidade = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,  // Pode duplicar
        (GDestroyNotify)g_ptr_array_unref);

    return g;
}

void gestor_passageiros_destruir(gestor_passageiros_t *gestor)
{
    if (!gestor)
        return;

    g_hash_table_destroy(gestor->por_documento);
    g_hash_table_destroy(gestor->por_nacionalidade);
    g_ptr_array_free(gestor->passageiros, TRUE);
    
    free(gestor);
}

void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *p)
{
    if (!gestor || !p)
        return;

    g_ptr_array_add(gestor->passageiros, p);

    // ⚡ Índice por documento (chave partilhada)
    const char *doc = passageiro_obter_document_number(p);
    if (doc)
        g_hash_table_insert(gestor->por_documento, (gpointer)doc, p);

    const char *nac = passageiro_obter_nacionalidade(p);
    if (nac)
    {
        GPtrArray *lista = g_hash_table_lookup(gestor->por_nacionalidade, nac);
        if (!lista)
        {
            lista = g_ptr_array_new();
            g_hash_table_insert(gestor->por_nacionalidade, g_strdup(nac), lista);
        }
        g_ptr_array_add(lista, p);
    }
}

passageiro_t *gestor_passageiros_obter_por_documento(
    gestor_passageiros_t *gestor,
    const char *document_number)
{
    if (!gestor || !document_number)
        return NULL;

    return g_hash_table_lookup(gestor->por_documento, document_number);
}

GPtrArray *gestor_passageiros_obter_por_nacionalidade(
    gestor_passageiros_t *gestor,
    const char *nacionalidade)
{
    if (!gestor || !nacionalidade)
        return NULL;

    return g_hash_table_lookup(gestor->por_nacionalidade, nacionalidade);
}

unsigned int gestor_passageiros_numero(gestor_passageiros_t *gestor)
{
    return gestor ? gestor->passageiros->len : 0;
}

static gboolean adiciona_passageiro_callback(void *contexto, void *objeto)
{
    gestor_passageiros_t *gestor = (gestor_passageiros_t *)contexto;
    passageiro_t *passageiro = (passageiro_t *)objeto;

    if (!gestor || !passageiro)
        return FALSE;

    gestor_passageiros_adicionar(gestor, passageiro);
    return TRUE;
}

void gestor_passageiros_carregar(gestor_passageiros_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    parser_carrega(
        gestor,
        ficheiro_csv,
        adiciona_passageiro_callback,
        (LinhaParaObjeto)valida_passageiro,
        (DestroiObjeto)passageiro_destruir);
}