#include "gestores/gestor_aeroportos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_aeroportos.h"
#include "entidades/aeroportos.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NUM_AEROPORTOS (26 * 26 * 26)

/**
 * @brief Estrutura que representa um gestor de aeroportos.
 *
 * Contém uma HashTable que mapeia códigos de aeroportos para as
 * estruturas correspondentes.
 */
struct gestor_aeroportos
{
    GHashTable *aeroportos;
    aeroporto_t **por_idx;
};

/**
 * @brief Cria um gestor de aeroportos.
 *
 * @return Ponteiro para o gestor criado.
 */
gestor_aeroportos_t *gestor_aeroportos_criar(void)
{
    gestor_aeroportos_t *g = malloc(sizeof(gestor_aeroportos_t));
    g->aeroportos = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)aeroporto_destruir
    );
    g->por_idx = g_malloc0(sizeof(aeroporto_t *) * NUM_AEROPORTOS);
    return g;
}

/**
 * @brief Destroi um gestor de aeroportos e libera a memória associada.
 *
 * @param gestor Ponteiro para o gestor a ser destruído.
 */
void gestor_aeroportos_destruir(gestor_aeroportos_t *gestor)
{
    if (!gestor)
        return;
    g_hash_table_destroy(gestor->aeroportos);
    g_free(gestor->por_idx);
    free(gestor);
}

/**
 * @brief Adiciona um aeroporto ao gestor.
 *
 * Se o aeroporto já existir, ele é destruído e não substitui o existente.
 *
 * @param gestor Ponteiro para o gestor de aeroportos.
 * @param aeroporto Ponteiro para o aeroporto a ser adicionado.
 */
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
    int idx = utils_aeroporto_index(id);
    if (idx >= 0 && idx < NUM_AEROPORTOS && gestor->por_idx)
        gestor->por_idx[idx] = aeroporto;
}

/**
 * @brief Obtém um aeroporto pelo seu ID.
 *
 * @param gestor Ponteiro para o gestor de aeroportos.
 * @param id Código do aeroporto.
 * @return Ponteiro para o aeroporto correspondente, ou NULL se não encontrado.
 */
aeroporto_t *gestor_aeroportos_obter_por_id(gestor_aeroportos_t *gestor, const char *id)
{
    if (!gestor || !id)
        return NULL;
    return g_hash_table_lookup(gestor->aeroportos, id);
}

/**
 * @brief Obtém um aeroporto pelo seu código.
 *
 * @param gestor Ponteiro para o gestor de aeroportos.
 * @param codigo Código do aeroporto.
 * @return Ponteiro para o aeroporto correspondente, ou NULL se não encontrado.
 */
aeroporto_t *gestor_aeroportos_obter_por_codigo(gestor_aeroportos_t *gestor, const char *codigo)
{
    if (!gestor || !codigo)
        return NULL;
    if (strlen(codigo) != 3)
        return NULL;
    int idx = utils_aeroporto_index(codigo);
    if (idx >= 0 && idx < NUM_AEROPORTOS && gestor->por_idx)
        return gestor->por_idx[idx];
    return g_hash_table_lookup(gestor->aeroportos, codigo);
}

aeroporto_t *gestor_aeroportos_obter_por_idx(gestor_aeroportos_t *gestor, int idx)
{
    if (!gestor || !gestor->por_idx)
        return NULL;
    if (idx < 0 || idx >= NUM_AEROPORTOS)
        return NULL;
    return gestor->por_idx[idx];
}

/**
 * @brief Retorna o número de aeroportos no gestor.
 *
 * @param gestor Ponteiro para o gestor de aeroportos.
 * @return Número de aeroportos armazenados.
 */
unsigned gestor_aeroportos_contar(const gestor_aeroportos_t *gestor)
{
    return gestor && gestor->aeroportos ? g_hash_table_size(gestor->aeroportos) : 0;
}

/**
 * @brief Executa uma função de callback para cada aeroporto do gestor.
 *
 * @param gestor Ponteiro para o gestor de aeroportos.
 * @param callback Função que será chamada para cada aeroporto.
 * @param user_data Dados do usuário que serão passados para o callback.
 */
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

/**
 * @brief Callback interno usado para adicionar aeroportos ao gestor durante o carregamento.
 *
 * @param contexto Ponteiro para o gestor de aeroportos.
 * @param objeto Ponteiro para o aeroporto a ser adicionado.
 * @return Sempre retorna TRUE para continuar a iteração.
 */
static gboolean _adiciona_aeroporto_callback(void *contexto, void *objeto)
{
    gestor_aeroportos_t *gestor = (gestor_aeroportos_t *)contexto;
    gestor_aeroportos_adicionar(gestor, (aeroporto_t *)objeto);
    return TRUE;
}

/**
 * @brief Carrega aeroportos de um ficheiro CSV e adiciona ao gestor.
 *
 * @param gestor Ponteiro para o gestor de aeroportos.
 * @param ficheiro_csv Caminho para o ficheiro CSV contendo os aeroportos.
 */
void gestor_aeroportos_carregar(gestor_aeroportos_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;
    parser_carrega(
        gestor,
        ficheiro_csv,
        _adiciona_aeroporto_callback,
        (LinhaParaObjeto)valida_aeroporto,
        (DestroiObjeto)aeroporto_destruir,
        8,
        8
    );
}
