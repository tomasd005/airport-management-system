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
 *
 * @return Ponteiro para o gestor criado.
 */
gestor_aeroportos_t *gestor_aeroportos_criar(void);

/**
 * @brief Destroi o gestor de aeroportos e liberta recursos.
 *
 * @param gestor Gestor de aeroportos.
 */
void gestor_aeroportos_destruir(gestor_aeroportos_t *gestor);

/**
 * @brief Adiciona um aeroporto ao gestor.
 *
 * @param gestor Gestor de aeroportos.
 * @param aeroporto Aeroporto a adicionar.
 */
void gestor_aeroportos_adicionar(gestor_aeroportos_t *gestor, aeroporto_t *aeroporto);

/**
 * @brief Obtém um aeroporto pelo código IATA.
 *
 * @param gestor Gestor de aeroportos.
 * @param codigo Código IATA.
 * @return Ponteiro para o aeroporto ou NULL se não existir.
 */
aeroporto_t *gestor_aeroportos_obter_por_codigo(gestor_aeroportos_t *gestor, const char *codigo);

/**
 * @brief Obtém um aeroporto pelo índice numérico.
 *
 * @param gestor Gestor de aeroportos.
 * @param indice Índice (0..17575).
 * @return Ponteiro para o aeroporto ou NULL se não existir.
 */
aeroporto_t *gestor_aeroportos_obter_por_indice(gestor_aeroportos_t *gestor, int indice);

/**
 * @brief Retorna o número de aeroportos no gestor.
 *
 * @param gestor Gestor de aeroportos.
 * @return Número de aeroportos.
 */
unsigned int gestor_aeroportos_numero(gestor_aeroportos_t *gestor);

/**
 * @brief Retorna o número de aeroportos no gestor (const).
 *
 * @param gestor Gestor de aeroportos.
 * @return Número de aeroportos.
 */
unsigned int gestor_aeroportos_contar(const gestor_aeroportos_t *gestor);

/**
 * @brief Retorna o total de aeroportos no gestor (alias).
 *
 * @param gestor Gestor de aeroportos.
 * @return Total de aeroportos.
 */
unsigned int gestor_aeroportos_total(const gestor_aeroportos_t *gestor);

/**
 * @brief Carrega aeroportos de um ficheiro CSV.
 *
 * @param gestor Gestor de aeroportos.
 * @param ficheiro_csv Caminho do CSV.
 */
void gestor_aeroportos_carregar(gestor_aeroportos_t *gestor, const char *ficheiro_csv);

/**
 * @brief Aplica um callback a cada aeroporto.
 *
 * @param gestor Gestor de aeroportos.
 * @param callback Função a executar por aeroporto.
 * @param user_data Dados do utilizador.
 */
void gestor_aeroportos_para_cada(gestor_aeroportos_t *gestor, void (*callback)(aeroporto_t *, void *), void *user_data);

#endif
