#ifndef VALIDACAO_AEROPORTOS_H
#define VALIDACAO_AEROPORTOS_H

#include <glib.h>

/**
 * @brief Valida os campos de um aeroporto a partir de colunas CSV.
 * 
 * Esta função verifica se todos os campos obrigatórios estão presentes,
 * se os códigos e tipos são válidos e se as coordenadas são consistentes.
 * Se os dados forem válidos, cria e retorna um ponteiro para a entidade aeroporto.
 * Caso contrário, retorna NULL.
 * 
 * @param colunas Array de strings com os valores das colunas do CSV.
 * @return Ponteiro para a entidade aeroporto_t se válido, NULL caso contrário.
 */
gpointer valida_aeroporto(char **colunas);

#endif