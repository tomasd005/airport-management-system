#ifndef QUERIE4_H
#define QUERIE4_H

#include "../gestores/gestor_reservas.h"
#include "../gestores/gestor_voos.h"
#include "../gestores/gestor_passageiros.h"
#include <stdio.h>

void query4(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *data_inicio,
            const char *data_fim,
            const char *comando_completo,
            FILE *output);

#endif
