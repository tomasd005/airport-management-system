#ifndef AVIOES_H
#define AVIOES_H

#include <stdbool.h>

/**
 * @file avioes.h
 * @brief Interface para manipulação de aviões.
 *
 * Define a estrutura e funções para criar, destruir e obter informações
 * sobre aviões, incluindo fabricante, modelo, capacidade e estatísticas de voos.
 */

/**
 * @brief Estrutura opaca que representa um avião.
 */
typedef struct aviao aviao_t;

/**
 * @brief Cria um avião.
 *
 * @param identificador Identificador único do avião.
 * @param fabricante Nome do fabricante do avião.
 * @param modelo Modelo do avião (pode ser string vazia).
 * @param ano Ano de fabricação (entre 1900 e o ano atual).
 * @param capacidade Número de assentos disponíveis.
 * @param alcance_km Alcance máximo do avião em quilômetros.
 * @return Ponteiro para o avião criado ou NULL em caso de erro.
 */
aviao_t *aviao_criar(const char *identificador, const char *fabricante, const char *modelo, int ano, int capacidade, int alcance_km);

/**
 * @brief Destrói um avião e libera a memória associada.
 *
 * @param a Ponteiro para o avião a destruir.
 */
void aviao_destruir(aviao_t *a);

/**
 * @brief Obtém o identificador do avião.
 *
 * @param a Avião.
 * @return Identificador ou NULL se a for NULL.
 */
const char *aviao_obter_identificador(const aviao_t *a);

/**
 * @brief Obtém o fabricante do avião.
 *
 * @param a Avião.
 * @return Fabricante ou NULL se a for NULL.
 */
const char *aviao_obter_fabricante(const aviao_t *a);

/**
 * @brief Obtém o modelo do avião.
 *
 * @param a Avião.
 * @return Modelo ou NULL se a for NULL.
 */
const char *aviao_obter_modelo(const aviao_t *a);

/**
 * @brief Obtém o ano de fabrico do avião.
 *
 * @param a Avião.
 * @return Ano de fabrico.
 */
int aviao_obter_ano(const aviao_t *a);

/**
 * @brief Obtém a capacidade do avião.
 *
 * @param a Avião.
 * @return Capacidade de assentos.
 */
int aviao_obter_capacidade(const aviao_t *a);

/**
 * @brief Obtém o alcance do avião em quilómetros.
 *
 * @param a Avião.
 * @return Alcance máximo em km.
 */
int aviao_obter_alcance_km(const aviao_t *a);

/**
 * @brief Obtém a contagem de voos associados ao avião.
 *
 * @param a Avião.
 * @return Número de voos.
 */
int aviao_obter_contagem_voos(const aviao_t *a);

/**
 * @brief Incrementa a contagem de voos do avião.
 *
 * @param a Avião.
 * @param delta Valor a somar (pode ser negativo).
 */
void aviao_incrementar_contagem_voos(aviao_t *a, int delta);

#endif
