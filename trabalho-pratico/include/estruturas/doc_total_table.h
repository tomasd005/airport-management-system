#ifndef DOC_TOTAL_TABLE_H
#define DOC_TOTAL_TABLE_H

#include <stddef.h>
#include <stdint.h>

/**
 * @file doc_total_table.h
 * @brief Tabela de dispersão compacta para acumular totais por documento.
 *
 * Implementa uma hash table com open addressing, usando chaves uint32_t e
 * valores double, otimizada para reduzir overhead de memória.
 */

typedef struct doc_total_table doc_total_table_t;

/**
 * @brief Cria uma nova tabela.
 * @param capacidade_inicial Capacidade inicial (será arredondada para potência de 2).
 * @return Ponteiro para a tabela criada.
 */
doc_total_table_t *doc_total_table_create(size_t capacidade_inicial);

/**
 * @brief Liberta a tabela e os seus recursos.
 * @param tabela Tabela a destruir.
 */
void doc_total_table_destroy(doc_total_table_t *tabela);

/**
 * @brief Adiciona um valor ao total associado à chave.
 * @param tabela Tabela.
 * @param key Chave do documento (uint32_t).
 * @param delta Valor a somar.
 */
void doc_total_table_add(doc_total_table_t *tabela, uint32_t key, double delta);

/**
 * @brief Itera por todas as entradas da tabela.
 * @param tabela Tabela.
 * @param callback Função chamada para cada par (key, total).
 * @param user_data Contexto do utilizador.
 */
void doc_total_table_foreach(doc_total_table_t *tabela,
                             void (*callback)(uint32_t key, double total, void *user_data),
                             void *user_data);

/**
 * @brief Obtém o número de entradas armazenadas.
 * @param tabela Tabela.
 * @return Número de chaves.
 */
size_t doc_total_table_size(const doc_total_table_t *tabela);

#endif
