#ifndef VALIDACAO_PASSAGEIROS_H
#define VALIDACAO_PASSAGEIROS_H

#include "../entidades/passageiros.h"
#include <glib.h>

/**
 * @brief Valida os campos de um passageiro.
 *
 * Esta função recebe um array de strings (colunas) representando
 * os campos de um passageiro e realiza todas as validações necessárias:
 * - Número de documento válido
 * - Nome, sobrenome, nacionalidade, telefone e endereço não vazios
 * - Data de nascimento no passado
 * - Gênero válido ('M', 'F' ou 'O')
 * - Email com formato válido
 *
 * @param colunas Array de strings com os campos do passageiro:
 * @return Ponteiro para `passageiro_t` criado se válido, NULL caso contrário.
 * @note Thread-safety: não thread-safe.
 */
passageiro_t *valida_passageiro(char **colunas);

/**
 * @brief Valida sintaticamente os campos do passageiro (formato).
 *
 * @param colunas Array de strings com os campos do passageiro
 * @return TRUE se sintaticamente válido.
 */
gboolean passageiro_validar_sintatica(char **colunas);

/**
 * @brief Valida logicamente os campos do passageiro (coerência).
 *
 * @param colunas Array de strings com os campos do passageiro
 * @return TRUE se logicamente válido.
 */
gboolean passageiro_validar_logica(char **colunas);

#endif
