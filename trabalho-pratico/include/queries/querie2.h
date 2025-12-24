#ifndef QUERIE2_H
#define QUERIE2_H

#include <stdio.h>
#include "../gestores/gestor_avioes.h"
#include "../gestores/gestor_voos.h"

void query2(gestor_avioes_t *gestor_avioes,
            gestor_voos_t *gestor_voos,
            int N,
            const char *fabricante,
            const char *comando_completo, // NOVO parâmetro
            FILE *output);
#endif
