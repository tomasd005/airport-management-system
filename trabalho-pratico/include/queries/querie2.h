#ifndef QUERIE2_H
#define QUERIE2_H

#include <stdio.h>

typedef struct gestor_avioes gestor_avioes_t;
typedef struct gestor_voos gestor_voos_t;

void query2(gestor_avioes_t *gestor_avioes,
            gestor_voos_t *gestor_voos,
            int N,
            const char *fabricante,
            const char *comando_completo, // NOVO parâmetro
            FILE *output);
#endif
