#ifndef VALIDACAO_PASSAGEIROS_H
#define VALIDACAO_PASSAGEIROS_H

#include "../entidades/passageiros.h"

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
 */
passageiro_t *valida_passageiro(char **colunas);

#endif