#ifndef QUERIE3_H
#define QUERIE3_H

#include <stdio.h>
#include "../../include/gestores/gestor_aeroportos.h"
#include "../../include/gestores/gestor_voos.h"


void query3(gestor_aeroportos_t *gestor_aeroportos,
            gestor_voos_t *gestor_voos,
            const char *data_inicio,
            const char *data_fim,
            FILE *output);

#endif 