#ifndef GESTOR_AVIOES_H
#define GESTOR_AVIOES_H

#include "../entidades/avioes.h"
#include <glib.h>

/**
 * @file gestor_avioes.h
 * @brief Gestão centralizada de aviões.
 *
 * Este módulo permite criar, destruir e gerir um conjunto de aviões,
 * incluindo inserção, consulta, carregamento a partir de CSV e iteração.
 */

typedef struct gestor_avioes gestor_avioes_t;

/**
 * @brief Cria um gestor de aviões.
 * @return Gestor alocado, ou NULL em erro de memória.
 */
gestor_avioes_t *gestor_avioes_criar(void);

/**
 * @brief Destrói o gestor e liberta todos os aviões.
 * @param gestor Gestor a destruir (aceita NULL).
 */
void gestor_avioes_destruir(gestor_avioes_t *gestor);

/**
 * @brief Adiciona um avião ao gestor.
 * @param gestor Gestor de aviões.
 * @param aviao Avião a inserir (ownership transferido).
 */
void gestor_avioes_adicionar(gestor_avioes_t *gestor, aviao_t *aviao);

/**
 * @brief Obtém avião por ID.
 * @param gestor Gestor de aviões.
 * @param identificador Identificador do avião.
 * @return Ponteiro para o avião, ou NULL se não existir.
 */
aviao_t *gestor_avioes_obter_por_id(gestor_avioes_t *gestor, const char *identificador);

/**
 * @brief Devolve o número de aviões.
 * @param gestor Gestor de aviões.
 * @return Total de aviões.
 */
unsigned int gestor_avioes_contar(const gestor_avioes_t *gestor);

/**
 * @brief Carrega aviões a partir de CSV.
 * @param gestor Gestor de aviões.
 * @param ficheiro_csv Caminho para o CSV.
 */
void gestor_avioes_carregar(gestor_avioes_t *gestor, const char *ficheiro_csv);

/**
 * @brief Itera por cada avião e invoca um callback.
 * @param gestor Gestor de aviões.
 * @param callback Função chamada para cada avião.
 * @param user_data Contexto do utilizador.
 */
void gestor_avioes_para_cada(gestor_avioes_t *gestor, void (*callback)(aviao_t *, void *), void *user_data);

#endif
