#ifndef QUERIE4_H
#define QUERIE4_H

#include <stdio.h>

typedef struct gestor_reservas gestor_reservas_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_passageiros gestor_passageiros_t;

void query4(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *data_inicio,
            const char *data_fim,
            const char *comando_completo,
            FILE *output);

#endif
