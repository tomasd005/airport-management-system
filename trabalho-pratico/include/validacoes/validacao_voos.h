#ifndef VALIDACAO_VOOS_H
#define VALIDACAO_VOOS_H

#include "../entidades/voos.h"

/**
 * @brief Valida uma linha CSV e cria um objeto voo_info.
 *
 * Recebe um array de strings representando os campos de um voo e valida:
 * - Flight ID
 * - Horários de partida e chegada (planejados e reais)
 * - Gate, status, origem e destino
 * - Aeronave, companhia aérea e URL de rastreamento
 *
 * @param colunas Array de strings com os campos do voo.
 * @return Ponteiro para `voo_info_t` se válido, NULL caso algum campo seja inválido.
 */
voo_info_t *valida_voo(char **colunas);

#endif
