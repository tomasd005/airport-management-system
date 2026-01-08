#ifndef QUERIE3_H
#define QUERIE3_H

#include <stdio.h>

typedef struct gestor_aeroportos gestor_aeroportos_t;
typedef struct gestor_voos gestor_voos_t;

void query3(gestor_aeroportos_t *gestor_aeroportos,
            gestor_voos_t *gestor_voos,
            const char *data_inicio,
            const char *data_fim,
            const char *comando_completo, // NOVO parâmetro
            FILE *output);

#endif
