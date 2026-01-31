#ifndef VALIDACAO_AVIOES_H
#define VALIDACAO_AVIOES_H

#include "../entidades/avioes.h"
#include <glib.h>

/**
 * @brief Valida os campos de um avião a partir de colunas CSV.
 * 
 * Esta função verifica se todos os campos obrigatórios estão presentes,
 * se os valores numéricos (ano, capacidade, alcance) são válidos e consistentes.
 * Se os dados forem válidos, cria e retorna um ponteiro para a entidade aviao_t.
 * Caso contrário, retorna NULL.
 * 
 * @param colunas Array de strings com os valores das colunas do CSV.
 * @return Ponteiro para a entidade aviao_t se válido, NULL caso contrário.
 */
aviao_t *valida_aviao(char **colunas);

/**
 * @brief Validação sintática dos campos do avião.
 */
gboolean aviao_validar_sintatica(char **colunas, int *out_ano, int *out_cap, int *out_alc,
                                 const char **out_modelo);

/**
 * @brief Validação lógica dos campos do avião (regras adicionais).
 */
gboolean aviao_validar_logica(char **colunas);

#endif
