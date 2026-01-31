#ifndef GESTOR_AEROPORTOS_H
#define GESTOR_AEROPORTOS_H

#include "../entidades/aeroportos.h"
#include <glib.h>

/**
 * @file gestor_aeroportos.h
 * @brief Gestão centralizada de aeroportos.
 *
 * Este módulo fornece funções para criar, destruir e gerir um conjunto
 * de aeroportos, permitindo inserção, consulta e iteração sobre os dados.
 */

typedef struct gestor_aeroportos gestor_aeroportos_t;

/**
 * @brief Cria um gestor de aeroportos.
 * @return Gestor alocado, ou NULL em erro de memória.
 */
gestor_aeroportos_t *gestor_aeroportos_criar(void);

/**
 * @brief Destrói o gestor e liberta todos os aeroportos.
 * @param gestor Gestor a destruir (aceita NULL).
 */
void gestor_aeroportos_destruir(gestor_aeroportos_t *gestor);

/**
 * @brief Adiciona um aeroporto ao gestor.
 * @param gestor Gestor de aeroportos.
 * @param aeroporto Aeroporto a inserir (ownership transferido).
 */
void gestor_aeroportos_adicionar(gestor_aeroportos_t *gestor, aeroporto_t *aeroporto);

/**
 * @brief Obtém aeroporto pelo código (ex.: "LIS").
 * @param gestor Gestor de aeroportos.
 * @param codigo Código do aeroporto.
 * @return Ponteiro para o aeroporto, ou NULL se não existir.
 */
aeroporto_t *gestor_aeroportos_obter_por_codigo(gestor_aeroportos_t *gestor, const char *codigo);

/**
 * @brief Obtém aeroporto pelo índice interno.
 * @param gestor Gestor de aeroportos.
 * @param idx Índice interno.
 * @return Ponteiro para o aeroporto, ou NULL se inválido.
 */
aeroporto_t *gestor_aeroportos_obter_por_idx(gestor_aeroportos_t *gestor, int idx);

/**
 * @brief Devolve o número de aeroportos carregados.
 * @param gestor Gestor de aeroportos.
 * @return Total de aeroportos.
 */
unsigned int gestor_aeroportos_numero(gestor_aeroportos_t *gestor);

/**
 * @brief Devolve o número de aeroportos (alias).
 * @param gestor Gestor de aeroportos.
 * @return Total de aeroportos.
 */
unsigned int gestor_aeroportos_contar(const gestor_aeroportos_t *gestor);

/**
 * @brief Devolve o número total de aeroportos (alias).
 * @param gestor Gestor de aeroportos.
 * @return Total de aeroportos.
 */
unsigned int gestor_aeroportos_total(const gestor_aeroportos_t *gestor);

/**
 * @brief Carrega aeroportos a partir de um CSV.
 * @param gestor Gestor de aeroportos.
 * @param ficheiro_csv Caminho para o ficheiro CSV.
 * @note Linhas inválidas são registadas nos erros.
 */
void gestor_aeroportos_carregar(gestor_aeroportos_t *gestor, const char *ficheiro_csv);

/**
 * @brief Itera por cada aeroporto e invoca um callback.
 * @param gestor Gestor de aeroportos.
 * @param callback Função chamada para cada aeroporto.
 * @param user_data Contexto do utilizador.
 */
void gestor_aeroportos_para_cada(gestor_aeroportos_t *gestor, void (*callback)(aeroporto_t *, void *), void *user_data);

#endif
