#ifndef VALIDACAO_RESERVAS_H
#define VALIDACAO_RESERVAS_H

#include <glib.h>
#include <stddef.h>
#include <stdint.h>
#include "../entidades/reservas.h"
#include "../gestores/gestor_voos.h"
#include "../gestores/gestor_passageiros.h"

/**
 * @brief Valida uma linha CSV e cria uma reserva.
 *
 * Recebe um array de strings representando os campos de uma reserva
 * e valida todos os dados: ID da reserva, IDs de voos, documento do passageiro,
 * assento, preço, bagagem extra, embarque prioritário e QR code.
 *
 * @param colunas Array de strings com os campos da reserva.
 * @return Ponteiro para `reserva_t` se válido, NULL caso contrário.
 */
reserva_t *valida_reserva_from_csv(char **colunas);

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

/**
 * @brief Valida logicamente uma reserva.
 *
 * Verifica consistência da reserva com gestores de voos e passageiros:
 * - Cada flight_id deve corresponder a um voo existente.
 * - O document_number deve corresponder a um passageiro existente.
 * - Se há dois voos, o destino do primeiro deve ser igual à origem do segundo.
 *
 * @param r Ponteiro para a reserva a validar.
 * @param gestor_voos Ponteiro para o gestor de voos.
 * @param gestor_passageiros Ponteiro para o gestor de passageiros.
 * @return Um GPtrArray contendo strings de mensagens de erro, ou NULL se não houver erros.
 */
GPtrArray *validar_reserva(const reserva_t *r, gestor_voos_t *gestor_voos,
                           gestor_passageiros_t *gestor_passageiros);

/**
 * @brief Libera a memória de um array de erros retornado por `validar_reserva`.
 *
 * @param erros GPtrArray contendo mensagens de erro.
 */
void validar_reserva_imprimir_erros(GPtrArray *erros);

#endif
