#ifndef VALIDACAO_RESERVAS_H
#define VALIDACAO_RESERVAS_H

#include <glib.h>
#include <stddef.h>
#include "../entidades/reservas.h"
#include "../gestores/gestor_voos.h"
#include "../gestores/gestor_passageiros.h"

reserva_t *valida_reserva_from_csv(char **colunas);
gboolean valida_reserva_campos(char **colunas,
                               const char **out_flight_ids,
                               size_t *out_num_voos,
                               const char **out_document_number,
                               double *out_preco);

GPtrArray *validar_reserva(const reserva_t *r,
                           gestor_voos_t *gestor_voos,
                           gestor_passageiros_t *gestor_passageiros);

void validar_reserva_imprimir_erros(GPtrArray *erros);

#endif
