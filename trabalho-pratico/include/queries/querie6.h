#ifndef QUERIE6_H
#define QUERIE6_H

#include <stdio.h>

typedef struct gestor_reservas gestor_reservas_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_passageiros gestor_passageiros_t;

void query6(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *nacionalidade,
            const char *comando_completo,
            FILE *output);

#endif
