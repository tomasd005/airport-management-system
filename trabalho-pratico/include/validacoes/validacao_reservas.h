#ifndef VALIDACAO_RESERVAS_H
#define VALIDACAO_RESERVAS_H

#include <glib.h>
#include <stddef.h>
#include <stdint.h>

typedef struct voo voo_t;
typedef struct passageiro passageiro_t;

/**
 * @brief Valida os campos de uma reserva sem criar o objeto.
 *
 * Permite extrair informações úteis da reserva (como IDs de voo, preço
 * e número de voos) para uso posterior.
 *
 * @param colunas Array de strings com os campos da reserva.
 * @param out_flight_ids Saída: ponteiros para IDs de voo validados.
 * @param out_num_voos Saída: número de voos válidos na reserva.
 * @param out_document_number Saída: documento do passageiro.
 * @param out_document_key Saída: chave numérica do documento.
 * @param out_preco Saída: preço da reserva.
 * @return TRUE se todos os campos forem válidos, FALSE caso contrário.
 * @note Thread-safety: não thread-safe.
 */
gboolean valida_reserva_campos(char **colunas, const char **out_flight_ids, size_t *out_num_voos,
                               const char **out_document_number, uint32_t *out_document_key,
                               double *out_preco);

/**
 * @brief Validação sintática de reserva (formato de campos).
 *
 * @param colunas Array de strings com os campos da reserva
 * @param out_flight_ids Array para retornar os flight IDs
 * @param out_num_voos Ponteiro para número de voos
 * @param out_document_number Ponteiro para número de documento
 * @param out_document_key Ponteiro para chave de documento
 * @param out_preco Ponteiro para preço
 * @return TRUE se sintaticamente válida
 * @note Ownership: ponteiros em out_* referenciam o buffer original das colunas.
 */
gboolean reserva_validar_sintatica(char **colunas, const char **out_flight_ids,
                                   size_t *out_num_voos, const char **out_document_number,
                                   uint32_t *out_document_key, double *out_preco);


/**
 * @brief Validação lógica de reserva (referências e coerência entre voos).
 *
 * @param voos Array de voos associados
 * @param num_voos Número de voos
 * @param passageiro Passageiro associado
 * @return TRUE se logicamente válida
 */
gboolean reserva_validar_logica(voo_t *const *voos, size_t num_voos, passageiro_t *passageiro);

#endif
