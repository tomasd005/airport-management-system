#ifndef VOO_TABLE_H
#define VOO_TABLE_H

#include <stddef.h>
#include <stdint.h>

/**
 * @file voo_table.h
 * @brief Tabela hash compacta para indexar voos por chave numérica.
 *
 * Implementa endereçamento aberto com linear probing. As chaves devem ser
 * diferentes de zero (0 é tratado como slot vazio).
 */

typedef struct voo voo_t;

/**
 * @struct voo_table_t
 * @brief Estrutura principal da tabela de voos.
 */
typedef struct {
    uint64_t *keys;
    voo_t **values;
    size_t size;
    size_t capacity;
} voo_table_t;

/**
 * @brief Inicializa a tabela com a capacidade mínima indicada.
 *
 * @param t Tabela a inicializar.
 * @param capacity Capacidade mínima desejada.
 */
void voo_table_init(voo_table_t *t, size_t capacity);

/**
 * @brief Liberta os recursos da tabela.
 *
 * @param t Tabela.
 * @param destroy Função para destruir valores (pode ser NULL).
 */
void voo_table_destroy(voo_table_t *t, void (*destroy)(voo_t *));

/**
 * @brief Procura um voo por chave.
 *
 * @param t Tabela.
 * @param key Chave numérica (não pode ser 0).
 * @return Ponteiro para voo_t ou NULL se não encontrado.
 */
voo_t *voo_table_lookup(const voo_table_t *t, uint64_t key);

/**
 * @brief Insere um voo na tabela.
 *
 * @param t Tabela.
 * @param key Chave numérica (não pode ser 0).
 * @param value Ponteiro para voo_t.
 * @return 1 em sucesso, 0 em erro ou chave duplicada.
 */
int voo_table_insert(voo_table_t *t, uint64_t key, voo_t *value);

/**
 * @brief Itera todos os voos armazenados.
 *
 * @param t Tabela.
 * @param fn Função callback.
 * @param user_data Contexto opcional.
 */
void voo_table_foreach(const voo_table_t *t, void (*fn)(voo_t *, void *), void *user_data);

/**
 * @brief Obtém o número de elementos armazenados.
 *
 * @param t Tabela.
 * @return Número de voos.
 */
size_t voo_table_size(const voo_table_t *t);

#endif
