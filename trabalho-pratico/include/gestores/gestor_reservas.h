#ifndef GESTOR_RESERVAS_H
#define GESTOR_RESERVAS_H

#include <glib.h>
#include "../entidades/reservas.h"

typedef struct gestor_reservas gestor_reservas_t;

gestor_reservas_t *gestor_reservas_criar(void);
void gestor_reservas_destruir(gestor_reservas_t *gestor);
void gestor_reservas_adicionar(gestor_reservas_t *gestor, reserva_t *reserva);
reserva_t *gestor_reservas_obter_por_id(gestor_reservas_t *gestor, const char *reservation_id);
unsigned int gestor_reservas_numero(gestor_reservas_t *gestor);

// Função de carregamento (antes em _parser.h)
void gestor_reservas_carregar(gestor_reservas_t *gestor, const char *ficheiro_csv);
// Adiciona esta declaração:
int gestor_reservas_contar_passageiros_voo(gestor_reservas_t *gestor, const char *flight_id);
#endif