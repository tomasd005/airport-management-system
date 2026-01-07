#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../../include/gestores/gestor_aeroportos.h"
#include "../../include/entidades/aeroportos.h"
#include "../../include/queries/querie1.h"

static inline int usa_formato_alternativo(const char *comando)
{
    if (!comando)
        return 0;
    while (*comando && isspace(*comando))
        comando++;
    while (*comando && isdigit(*comando))
        comando++;
    return (*comando == 'S');
}

void query1(gestor_aeroportos_t *gestor_aeroportos,
            gestor_voos_t *gestor_voos,
            gestor_reservas_t *gestor_reservas,
            const char *comando_completo,
            const char *airport_code,
            FILE *output)
{
    if (!gestor_aeroportos || !airport_code || !output)
    {
        fprintf(output, "\n");
        return;
    }

    char clean_code[16];
    snprintf(clean_code, sizeof(clean_code), "%s", airport_code);
    clean_code[strcspn(clean_code, "\r\n ")] = '\0';

    aeroporto_t *aeroporto = gestor_aeroportos_obter_por_codigo(gestor_aeroportos, clean_code);
    if (!aeroporto)
    {
        fprintf(output, "\n");
        return;
    }

    (void)gestor_voos;
    (void)gestor_reservas;

    int arrival_count = aeroporto_obter_chegadas(aeroporto);
    int departure_count = aeroporto_obter_partidas(aeroporto);

    const char *separador = usa_formato_alternativo(comando_completo) ? "=" : ";";

    fprintf(output, "%s%s%s%s%s%s%s%s%s%s%d%s%d\n",
            aeroporto_obter_codigo(aeroporto), separador,
            aeroporto_obter_nome(aeroporto), separador,
            aeroporto_obter_cidade(aeroporto), separador,
            aeroporto_obter_pais(aeroporto), separador,
            aeroporto_obter_tipo(aeroporto), separador,
            arrival_count, separador,
            departure_count);
}
