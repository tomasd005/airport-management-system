#include "../../include/queries/querie5.h"
#include "../../include/gestores/gestor_voos.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * @brief Verifica se o comando indica uso do formato alternativo.
 *
 * O formato alternativo é indicado quando, após dígitos iniciais e espaços,
 * aparece 'S'.
 *
 * @param cmd Comando completo.
 * @return 1 se usar formato alternativo, 0 caso contrário.
 */
static inline int usa_formato_alternativo(const char *cmd)
{
    while (*cmd && isspace(*cmd))
        cmd++;
    while (*cmd && isdigit(*cmd))
        cmd++;
    return (*cmd == 'S');
}

/**
 * @brief Executa a Query 5.
 *
 * @param gestor_voos Gestor de voos.
 * @param N Número máximo de companhias a imprimir.
 * @param comando_completo Comando completo, usado para definir formato alternativo.
 * @param output Ponteiro para arquivo onde será escrita a saída.
 */
void query5(gestor_voos_t *gestor_voos, int N, const char *comando_completo, FILE *output)
{
    if (!gestor_voos || !output || N <= 0)
    {
        fprintf(output, "\n");
        return;
    }

    const char *sep = usa_formato_alternativo(comando_completo) ? "=" : ";";

    const GArray *res = gestor_voos_obter_q5_cache(gestor_voos);
    if (!res || res->len == 0)
    {
        fprintf(output, "\n");
        return;
    }

    guint limite = res->len < (guint)N ? res->len : (guint)N;
    for (guint i = 0; i < limite; i++)
    {
        gestor_voos_q5_t *r = &g_array_index((GArray *)res, gestor_voos_q5_t, i);
        fprintf(output, "%s%s%u%s%.3f\n", r->airline, sep, r->count, sep, r->avg_delay);
    }
}
