#include "gestores/gestor_avioes.h"
#include "parsers/parser.h"
#include "validacoes/validacao_avioes.h"
#include "entidades/avioes.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Estrutura que representa um gestor de aviões.
 *
 * Contém uma HashTable que mapeia identificadores de aviões para
 * as estruturas correspondentes.
 */
struct gestor_avioes
{
    GHashTable *tabela;
};

/**
 * @brief Cria um gestor de aviões.
 *
 * @return Ponteiro para o gestor criado.
 */
gestor_avioes_t *gestor_avioes_criar(void)
{
    gestor_avioes_t *g = malloc(sizeof(gestor_avioes_t));
    g->tabela = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)aviao_destruir
    );
    return g;
}

/**
 * @brief Destroi um gestor de aviões e libera a memória associada.
 *
 * @param gestor Ponteiro para o gestor a ser destruído.
 */
void gestor_avioes_destruir(gestor_avioes_t *gestor)
{
    if (!gestor)
        return;
    g_hash_table_destroy(gestor->tabela);
    free(gestor);
}

/**
 * @brief Adiciona um avião ao gestor.
 *
 * Se o avião já existir no gestor, ele é destruído e não substitui o existente.
 *
 * @param gestor Ponteiro para o gestor de aviões.
 * @param aviao Ponteiro para o avião a ser adicionado.
 */
void gestor_avioes_adicionar(gestor_avioes_t *gestor, aviao_t *aviao)
{
    if (!gestor || !aviao)
        return;

    const char *id = aviao_obter_identificador(aviao);
    if (!id || g_hash_table_contains(gestor->tabela, id))
    {
        aviao_destruir(aviao);
        return;
    }

    g_hash_table_insert(gestor->tabela, g_strdup(id), aviao);
}

/**
 * @brief Obtém um avião pelo seu identificador.
 *
 * @param gestor Ponteiro para o gestor de aviões.
 * @param id Identificador do avião.
 * @return Ponteiro para o avião correspondente, ou NULL se não encontrado.
 */
aviao_t *gestor_avioes_obter_por_id(gestor_avioes_t *gestor, const char *id)
{
    return (gestor && id) ? g_hash_table_lookup(gestor->tabela, id) : NULL;
}

/**
 * @brief Retorna o número de aviões no gestor.
 *
 * @param gestor Ponteiro para o gestor de aviões.
 * @return Número de aviões armazenados.
 */
unsigned gestor_avioes_contar(const gestor_avioes_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->tabela) : 0;
}

/**
 * @brief Executa uma função de callback para cada avião do gestor.
 *
 * @param gestor Ponteiro para o gestor de aviões.
 * @param func Função que será chamada para cada avião.
 * @param user_data Dados do usuário que serão passados para o callback.
 */
void gestor_avioes_para_cada(gestor_avioes_t *gestor, void (*func)(aviao_t *, void *), void *user_data)
{
    if (!gestor || !func)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
        func(value, user_data);
}

/**
 * @brief Callback interno usado para adicionar aviões ao gestor durante o carregamento.
 *
 * @param contexto Ponteiro para o gestor de aviões.
 * @param objeto Ponteiro para o avião a ser adicionado.
 * @return Sempre retorna TRUE para continuar a iteração.
 */
static gboolean _adiciona_aviao_callback(void *contexto, void *objeto)
{
    gestor_avioes_adicionar(contexto, objeto);
    return TRUE;
}

/**
 * @brief Carrega aviões de um ficheiro CSV e adiciona ao gestor.
 *
 * @param gestor Ponteiro para o gestor de aviões.
 * @param ficheiro_csv Caminho para o ficheiro CSV contendo os aviões.
 */
void gestor_avioes_carregar(gestor_avioes_t *gestor, const char *ficheiro_csv)
{
    if (gestor && ficheiro_csv)
    parser_carrega(
        gestor,
        ficheiro_csv,
        _adiciona_aviao_callback,
        (LinhaParaObjeto)valida_aviao,
        (DestroiObjeto)aviao_destruir,
        6,
        6
    );
}
