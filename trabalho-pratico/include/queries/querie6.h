#ifndef QUERIE6_H
#define QUERIE6_H

#include <stdio.h>
#include "../gestores/gestor_reservas.h"
#include "../gestores/gestor_voos.h"
#include "../gestores/gestor_passageiros.h"

void query6(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *nacionalidade,
            const char *comando_completo,
            FILE *output);

#endif