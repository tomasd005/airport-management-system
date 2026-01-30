#ifndef VALIDACAO_RESERVAS_H
#define VALIDACAO_RESERVAS_H

#include <glib.h>
#include <stddef.h>
#include <stdint.h>

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
 */
gboolean valida_reserva_campos(char **colunas, const char **out_flight_ids, size_t *out_num_voos,
                               const char **out_document_number, uint32_t *out_document_key,
                               double *out_preco);

#endif
