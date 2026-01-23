#include "gestores/gestor_passageiros.h"
#include "parsers/parser.h"
#include "validacoes/validacao_passageiros.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Estrutura que representa um gestor de passageiros.
 *
 * Contém uma HashTable que mapeia números de documento para as
 * estruturas de passageiros correspondentes.
 */
struct gestor_passageiros
{
    GHashTable *por_documento;
};

/**
 * @brief Cria um gestor de passageiros.
 *
 * @return Ponteiro para o gestor criado.
 */
gestor_passageiros_t *gestor_passageiros_criar(void)
{
    gestor_passageiros_t *g = malloc(sizeof(*g));
    g->por_documento = g_hash_table_new_full(
        g_direct_hash,
        g_direct_equal,
        NULL,
        (GDestroyNotify)passageiro_destruir
    );
    return g;
}

/**
 * @brief Destroi um gestor de passageiros e libera a memória associada.
 *
 * @param gestor Ponteiro para o gestor a ser destruído.
 */
void gestor_passageiros_destruir(gestor_passageiros_t *gestor)
{
    if (!gestor)
        return;
    g_hash_table_destroy(gestor->por_documento);
    free(gestor);
}

/**
 * @brief Adiciona um passageiro ao gestor.
 *
 * Se o passageiro já existir no gestor (mesmo número de documento),
 * é destruído e não substitui o existente.
 *
 * @param gestor Ponteiro para o gestor de passageiros.
 * @param p Ponteiro para o passageiro a ser adicionado.
 */
void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *p)
{
    if (!gestor || !p)
        return;

    const char *doc = passageiro_obter_document_number(p);
    uint32_t key = 0;
    if (!doc || !utils_document_number_key(doc, &key))
    {
        passageiro_destruir(p);
        return;
    }

    gpointer kptr = GINT_TO_POINTER((gint)(key + 1u));
    if (g_hash_table_contains(gestor->por_documento, kptr))
    {
        passageiro_destruir(p);
        return;
    }

    g_hash_table_insert(gestor->por_documento, kptr, p);
}

/**
 * @brief Obtém um passageiro pelo número de documento.
 *
 * @param gestor Ponteiro para o gestor de passageiros.
 * @param document_number Número do documento do passageiro.
 * @return Ponteiro para o passageiro correspondente, ou NULL se não encontrado.
 */
passageiro_t *gestor_passageiros_obter_por_documento(gestor_passageiros_t *gestor, const char *document_number)
{
    uint32_t key = 0;
    if (!gestor || !document_number || !utils_document_number_key(document_number, &key))
        return NULL;
    return gestor_passageiros_obter_por_documento_key(gestor, key);
}

passageiro_t *gestor_passageiros_obter_por_documento_key(gestor_passageiros_t *gestor, uint32_t key)
{
    if (!gestor)
        return NULL;
    return g_hash_table_lookup(gestor->por_documento, GINT_TO_POINTER((gint)(key + 1u)));
}

/**
 * @brief Retorna o número de passageiros no gestor.
 *
 * @param gestor Ponteiro para o gestor de passageiros.
 * @return Número de passageiros armazenados.
 */
unsigned int gestor_passageiros_numero(gestor_passageiros_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->por_documento) : 0;
}

/**
 * @brief Callback interno usado para adicionar passageiros ao gestor durante o carregamento.
 *
 * @param contexto Ponteiro para o gestor de passageiros.
 * @param objeto Ponteiro para o passageiro a ser adicionado.
 * @return Sempre retorna TRUE para continuar a iteração.
 */
static gboolean adiciona_passageiro_callback(void *contexto, void *objeto)
{
    if (contexto && objeto)
        gestor_passageiros_adicionar(contexto, objeto);
    return TRUE;
}

/**
 * @brief Carrega passageiros de um ficheiro CSV e adiciona ao gestor.
 *
 * @param gestor Ponteiro para o gestor de passageiros.
 * @param ficheiro_csv Caminho para o ficheiro CSV contendo os passageiros.
 */
void gestor_passageiros_carregar(gestor_passageiros_t *gestor, const char *ficheiro_csv)
{
    if (gestor && ficheiro_csv)
        parser_carrega(
            gestor,
            ficheiro_csv,
            adiciona_passageiro_callback,
            (LinhaParaObjeto)valida_passageiro,
            (DestroiObjeto)passageiro_destruir,
            10,
            10
        );
}

/**
 * @brief Executa uma função de callback para cada passageiro do gestor.
 *
 * @param gestor Ponteiro para o gestor de passageiros.
 * @param func Função que será chamada para cada passageiro.
 * @param user_data Dados do usuário que serão passados para o callback.
 */
void gestor_passageiros_para_cada(gestor_passageiros_t *gestor, void (*func)(passageiro_t *, void *), void *user_data)
{
    if (!gestor || !func)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->por_documento);

    while (g_hash_table_iter_next(&iter, &key, &value))
        func(value, user_data);
}
