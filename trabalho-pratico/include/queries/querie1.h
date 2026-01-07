#ifndef QUERY1_H
#define QUERY1_H

#include "../gestores/gestor_aeroportos.h"
#include "../gestores/gestor_voos.h"
#include "../gestores/gestor_reservas.h"
#include <stdio.h>

void query1(gestor_aeroportos_t *gestor_aeroportos,
            gestor_voos_t *gestor_voos,
            gestor_reservas_t *gestor_reservas,
            const char *comando_completo,
            const char *airport_code,
            FILE *output);
#endif
