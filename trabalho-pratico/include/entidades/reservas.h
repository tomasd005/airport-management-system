#ifndef RESERVAS_H
#define RESERVAS_H

#include <stdbool.h>
#include <stddef.h>

typedef struct reserva reserva_t;

reserva_t *reserva_criar(const char *reservation_id, const char **flight_ids, size_t num_flights, const char *document_number, const char *seat, double preco, bool extra_bagagem, bool embarque_prioritario, const char *qr_code);

void reserva_destruir(reserva_t *r);

const char *reserva_obter_id(const reserva_t *r);
size_t reserva_obter_num_voos(const reserva_t *r);
const char **reserva_obter_flight_ids(const reserva_t *r);
const char *reserva_obter_document_number(const reserva_t *r);
const char *reserva_obter_seat(const reserva_t *r);
double reserva_obter_preco(const reserva_t *r);
bool reserva_obter_extra_bagagem(const reserva_t *r);
bool reserva_obter_embarque_prioritario(const reserva_t *r);
const char *reserva_obter_qr_code(const reserva_t *r);
/* Para query6 */
size_t reserva_obter_num_passageiros(const reserva_t *r);
const char **reserva_obter_documentos(const reserva_t *r);

#endif
