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
 * @note Thread-safety: não thread-safe.
 */
gpointer valida_aeroporto(char **colunas);

/**
 * @brief Validação sintática dos campos de aeroporto.
 */
gboolean aeroporto_validar_sintatica(char **colunas);

/**
 * @brief Validação lógica dos campos de aeroporto (coordenadas/tipo).
 *
 * @param colunas Array de strings com os valores das colunas.
 * @param out_lat Latitude validada.
 * @param out_lon Longitude validada.
 * @return TRUE se válido.
 */
gboolean aeroporto_validar_logica(char **colunas, double *out_lat, double *out_lon);

#endif
