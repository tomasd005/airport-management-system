#ifndef STRING_POOL_H
#define STRING_POOL_H

#include <stdint.h>

/**
 * @brief Interna uma string e devolve o seu identificador estável.
 *
 * @param s String a internar.
 * @return ID interno (>0) ou 0 para string vazia/NULL.
 * @note Ownership: a string internada é gerida pelo pool.
 * @note Thread-safety: sim (mutex interno).
 */
uint32_t string_pool_intern_id(const char *s);

/**
 * @brief Obtém a string internada por ID.
 *
 * @param id Identificador da string.
 * @return Ponteiro constante para a string, ou NULL se inválido.
 * @note Ownership: ponteiro válido enquanto o pool existir.
 */
const char *string_pool_get(uint32_t id);

/**
 * @brief Interna e devolve o ponteiro estável para a string.
 *
 * @param s String a internar.
 * @return Ponteiro estável para a string ("" se NULL/vazia).
 * @note Ownership: ponteiro válido enquanto o pool existir.
 */
const char *string_pool_intern(const char *s);

/**
 * @brief Limpa o pool (liberta toda a memória internada).
 *
 * Deve ser chamado no final da aplicação se for necessário libertar memória.
 * @note Thread-safety: não chamar concorrentemente com outras operações do pool.
 */
void string_pool_clear(void);

#endif
