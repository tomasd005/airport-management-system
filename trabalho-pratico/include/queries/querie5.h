#ifndef QUERIE5_H
#define QUERIE5_H

#include <stdio.h>
#include "../gestores/gestor_voos.h"


void query5(
    gestor_voos_t *gestor_voos,
    int N,
    const char *comando_completo,
    FILE *output);

#endif
