#include "gestores/gestor_passageiros.h"
#include "parsers/parser.h"
#include "validacoes/validacao_passageiros.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct gestor_passageiros
{
    GArray *passageiros;
    GHashTable *por_documento;     // NOVO: document_number -> passageiro_t*
    GHashTable *por_nacionalidade; // NOVO: nacionalidade -> GPtrArray de passageiros
};

gestor_passageiros_t *gestor_passageiros_criar(void)
{
    gestor_passageiros_t *g = malloc(sizeof(gestor_passageiros_t));
    g->passageiros = g_array_new(FALSE, FALSE, sizeof(passageiro_t *));

    // Hash por documento (O(1) lookup)
    g->por_documento = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        NULL,  // Não liberar chave (pertence ao passageiro)
        NULL); // Não liberar valor (gerido por passageiros array)

    // Hash por nacionalidade (O(1) lookup)
    g->por_nacionalidade = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)g_ptr_array_unref);

    return g;
}

void gestor_passageiros_destruir(gestor_passageiros_t *gestor)
{
    if (!gestor)
        return;

    for (guint i = 0; i < gestor->passageiros->len; i++)
        passageiro_destruir(g_array_index(gestor->passageiros, passageiro_t *, i));

    g_array_free(gestor->passageiros, TRUE);
    g_hash_table_destroy(gestor->por_documento);
    g_hash_table_destroy(gestor->por_nacionalidade);
    free(gestor);
}

void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *p)
{
    if (!gestor || !p)
        return;

    g_array_append_val(gestor->passageiros, p);

    // Adicionar ao índice por documento
    const char *doc = passageiro_obter_document_number(p);
    if (doc)
        g_hash_table_insert(gestor->por_documento, (gpointer)doc, p);

    // Adicionar ao índice por nacionalidade
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

// AGORA É O(1) em vez de O(n)!
passageiro_t *gestor_passageiros_obter_por_documento(
    gestor_passageiros_t *gestor,
    const char *document_number)
{
    if (!gestor || !document_number)
        return NULL;

    return g_hash_table_lookup(gestor->por_documento, document_number);
}

// NOVA FUNÇÃO: Obter passageiros por nacionalidade O(1)
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