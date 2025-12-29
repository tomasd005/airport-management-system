#ifndef VOOS_H
#define VOOS_H

#include <stdbool.h>

typedef struct voo voo_t;

voo_t *voo_criar(const char *flight_id, const char *departure, const char *actual_departure, const char *arrival, const char *actual_arrival, const char *gate, const char *status, const char *origin, const char *destination, const char *aircraft, const char *airline, const char *tracking_url);

void voo_destruir(voo_t *v);

const char *voo_obter_id(const voo_t *v);
const char *voo_obter_departure(const voo_t *v);
const char *voo_obter_actual_departure(const voo_t *v);
const char *voo_obter_arrival(const voo_t *v);
const char *voo_obter_actual_arrival(const voo_t *v);
const char *voo_obter_gate(const voo_t *v);
const char *voo_obter_status(const voo_t *v);
const char *voo_obter_origin(const voo_t *v);
const char *voo_obter_destination(const voo_t *v);
const char *voo_obter_aircraft(const voo_t *v);
const char *voo_obter_airline(const voo_t *v);
const char *voo_obter_tracking_url(const voo_t *v);
double voo_calcular_atraso_minutos(const voo_t *v);

#endif