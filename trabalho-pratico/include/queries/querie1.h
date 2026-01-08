#ifndef QUERY1_H
#define QUERY1_H

#include <stdio.h>

typedef struct gestor_aeroportos gestor_aeroportos_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_reservas gestor_reservas_t;

void query1(gestor_aeroportos_t *gestor_aeroportos,
            gestor_voos_t *gestor_voos,
            gestor_reservas_t *gestor_reservas,
            const char *comando_completo,
            const char *airport_code,
            FILE *output);
#endif
