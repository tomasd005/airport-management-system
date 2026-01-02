#ifndef GESTOR_RESERVAS_H
#define GESTOR_RESERVAS_H

#include "../entidades/reservas.h"
#include "gestor_voos.h"
#include <glib.h>  
typedef struct gestor_reservas gestor_reservas_t;
typedef struct gestor_passageiros gestor_passageiros_t;

gestor_reservas_t *gestor_reservas_criar(void);
void gestor_reservas_destruir(gestor_reservas_t *gestor);
void gestor_reservas_adicionar(gestor_reservas_t *gestor, reserva_t *r);
reserva_t *gestor_reservas_obter_por_id(gestor_reservas_t *gestor, const char *reservation_id);
int gestor_reservas_contar_passageiros_voo(gestor_reservas_t *gestor, const char *flight_id);
unsigned int gestor_reservas_numero(gestor_reservas_t *gestor);

GPtrArray *gestor_reservas_obter_por_passageiro(
    gestor_reservas_t *gestor,
    const char *document_number);

void gestor_reservas_carregar(gestor_reservas_t *gestor, const char *ficheiro_csv);

void gestor_reservas_carregar_com_validacao(
    gestor_reservas_t *gestor, 
    const char *ficheiro_csv,
    gestor_voos_t *gestor_voos,
    gestor_passageiros_t *gestor_passageiros);

void gestor_reservas_para_cada_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    const char *data_inicio,
    const char *data_fim,
    void (*callback)(int, const reserva_t *, void *),
    void *user_data);

void gestor_reservas_para_cada(
    gestor_reservas_t *gestor,
    void (*callback)(const reserva_t *, void *),
    void *user_data);

void gestor_reservas_para_cada_com_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    void (*callback)(int semana, const reserva_t *, void *),
    void *user_data);

#endif