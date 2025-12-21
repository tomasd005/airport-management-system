#include <stdio.h>
#include <string.h>
#include "../../include/gestores/gestor_aeroportos.h"
#include "../../include/entidades/aeroportos.h"
#include "../../include/queries/querie1.h"

void query1(gestor_aeroportos_t *gestor_aeroportos, const char *airport_code, FILE *output)
{
    if (!gestor_aeroportos || !airport_code || !output)
    {
        return;
    }

    char clean_code[16];
    snprintf(clean_code, sizeof(clean_code), "%s", airport_code);
    clean_code[strcspn(clean_code, "\r\n ")] = '\0';

    aeroporto_t *aeroporto = gestor_aeroportos_obter_por_codigo(gestor_aeroportos, clean_code);

    if (aeroporto)
    {
        fprintf(output, "%s,%s,%s,%s,%s\n",
                aeroporto_obter_codigo(aeroporto),
                aeroporto_obter_nome(aeroporto),
                aeroporto_obter_cidade(aeroporto),
                aeroporto_obter_pais(aeroporto),
                aeroporto_obter_tipo(aeroporto));
    }
    else
    {
        fprintf(output, "\n");
    }
}
