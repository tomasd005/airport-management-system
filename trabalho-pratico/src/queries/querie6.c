#include "../../include/queries/querie6.h"
#include "../../include/gestores/gestor_reservas.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Verifica se o comando indica uso do formato alternativo.
 *
 * O formato alternativo é indicado quando, após dígitos e espaços iniciais,
 * aparece 'S' ou 's', seguido de espaço, tabulação ou fim da string.
 *
 * @param cmd Comando completo.
 * @return 1 se usar formato alternativo, 0 caso contrário.
 */
static inline int usa_formato_alternativo(const char *cmd)
{
    if (!cmd)
        return 0;
    const char *p = cmd;
    while (*p && isspace(*p))
        p++;
    while (*p && isdigit(*p))
        p++;
    while (*p && isspace(*p))
        p++;
    if ((*p == 'S' || *p == 's') && (*(p + 1) == ' ' || *(p + 1) == '\t' || *(p + 1) == '\0'))
        return 1;
    return 0;
}

/**
 * @brief Executa a Query 6.
 *
 * @param gestor_reservas Gestor de reservas.
 * @param gestor_voos Gestor de voos (não usado nesta query, pode ser NULL).
 * @param gestor_passageiros Gestor de passageiros (não usado nesta query, pode ser NULL).
 * @param nacionalidade Nacionalidade dos passageiros a filtrar.
 * @param comando_completo Comando completo, usado para definir formato alternativo.
 * @param output Ponteiro para arquivo onde será escrita a saída.
 */
void query6(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *nacionalidade,
            const char *comando_completo,
            FILE *output)
{
    if (!gestor_reservas || !output || !nacionalidade || !*nacionalidade)
    {
        fprintf(output, "\n");
        return;
    }

    (void)gestor_voos;
    (void)gestor_passageiros;

    const char *sep = usa_formato_alternativo(comando_completo) ? "=" : ";";

    const char *destino = NULL;
    guint count = 0;
    if (!gestor_reservas_obter_melhor_destino_nacionalidade(gestor_reservas, nacionalidade, &destino, &count))
    {
        fprintf(output, "\n");
        return;
    }

    fprintf(output, "%s%s%u\n", destino, sep, count);
}
