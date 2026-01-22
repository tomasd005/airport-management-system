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
 *
 * @return Ponteiro para o gestor criado.
 */
gestor_avioes_t *gestor_avioes_criar(void);

/**
 * @brief Destroi o gestor de aviões e liberta recursos.
 *
 * @param gestor Gestor de aviões.
 */
void gestor_avioes_destruir(gestor_avioes_t *gestor);

/**
 * @brief Adiciona um avião ao gestor.
 *
 * @param gestor Gestor de aviões.
 * @param aviao Avião a adicionar.
 */
void gestor_avioes_adicionar(gestor_avioes_t *gestor, aviao_t *aviao);

/**
 * @brief Obtém um avião pelo identificador.
 *
 * @param gestor Gestor de aviões.
 * @param identificador Identificador do avião.
 * @return Ponteiro para o avião ou NULL se não existir.
 */
aviao_t *gestor_avioes_obter_por_id(gestor_avioes_t *gestor, const char *identificador);

/**
 * @brief Retorna o número de aviões no gestor.
 *
 * @param gestor Gestor de aviões.
 * @return Número de aviões.
 */
unsigned int gestor_avioes_contar(const gestor_avioes_t *gestor);

/**
 * @brief Carrega aviões de um ficheiro CSV.
 *
 * @param gestor Gestor de aviões.
 * @param ficheiro_csv Caminho do CSV.
 */
void gestor_avioes_carregar(gestor_avioes_t *gestor, const char *ficheiro_csv);

/**
 * @brief Aplica um callback a cada avião.
 *
 * @param gestor Gestor de aviões.
 * @param callback Função a executar por avião.
 * @param user_data Dados do utilizador.
 */
void gestor_avioes_para_cada(gestor_avioes_t *gestor, void (*callback)(aviao_t *, void *), void *user_data);

#endif
