#include "../../include/queries/querie2.h"
#include "../../include/gestores/gestor_avioes.h"
#include "../../include/entidades/avioes.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * @brief Verifica se o comando indica uso do formato alternativo.
 *
 * O formato alternativo é indicado quando, após dígitos iniciais, existe
 * a letra 'S'.
 *
 * @param comando Comando completo.
 * @return 1 se usar formato alternativo, 0 caso contrário.
 */
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

/**
 * @brief Executa a Query 2.
 *
 * @param gestor_avioes Gestor de aviões.
 * @param gestor_voos Gestor de voos (não utilizado nesta query).
 * @param N Número máximo de aviões a listar.
 * @param fabricante Filtro opcional de fabricante. NULL ou string vazia significa sem filtro.
 * @param comando_completo Comando completo, usado para definir formato alternativo.
 * @param output Ponteiro para arquivo onde será escrita a saída.
 */
void query2(gestor_avioes_t *gestor_avioes, gestor_voos_t *gestor_voos,
            int N, const char *fabricante, const char *comando_completo, FILE *output)
{
    if (!gestor_avioes || !output || N <= 0)
    {
        fprintf(output, "\n");
        return;
    }

    (void)gestor_voos;

    const char *separador = usa_formato_alternativo(comando_completo) ? "=" : ";";

    const GArray *resultados = gestor_avioes_obter_q2_ranking(gestor_avioes, fabricante);
    guint len = resultados ? resultados->len : 0;
    guint n_imprimir = (guint)N < len ? (guint)N : len;

    if (n_imprimir == 0)
        fprintf(output, "\n");
    else
    {
        for (guint i = 0; i < n_imprimir; i++)
        {
            const gestor_avioes_q2_item_t *c =
                &g_array_index((GArray *)resultados, gestor_avioes_q2_item_t, i);
            fprintf(output, "%s%s%s%s%s%s%u\n", c->id, separador, c->fabricante, separador, c->modelo, separador, c->count);
        }
    }
}
