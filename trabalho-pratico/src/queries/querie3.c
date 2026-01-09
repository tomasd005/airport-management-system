#include "../../include/queries/querie3.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/gestores/gestor_aeroportos.h"
#include "../../include/entidades/voos.h"
#include "../../include/entidades/aeroportos.h"
#include "../../include/utils.h"
#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

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
 * @brief Executa a Query 3.
 *
 * @param gestor_aeroportos Gestor de aeroportos.
 * @param gestor_voos Gestor de voos.
 * @param data_inicio Data de início no formato "YYYY-MM-DD".
 * @param data_fim Data de fim no formato "YYYY-MM-DD".
 * @param comando_completo Comando completo, usado para definir formato alternativo.
 * @param output Ponteiro para arquivo onde será escrita a saída.
 */
void query3(gestor_aeroportos_t *gestor_aeroportos,
            gestor_voos_t *gestor_voos,
            const char *data_inicio,
            const char *data_fim,
            const char *comando_completo,
            FILE *output)
{
    if (!gestor_aeroportos || !gestor_voos || !data_inicio || !data_fim || !output)
    {
        fprintf(output, "\n");
        return;
    }

    const char *separador = usa_formato_alternativo(comando_completo) ? "=" : ";";
    int dia_inicio = utils_parse_date_to_day(data_inicio);
    int dia_fim = utils_parse_date_to_day(data_fim);
    if (dia_inicio < 0 || dia_fim < 0 || dia_inicio > dia_fim)
    {
        fprintf(output, "\n");
        return;
    }

    const char *origem = NULL;
    guint contagem = 0;
    if (!gestor_voos_melhor_origem_intervalo(gestor_voos, dia_inicio, dia_fim, &origem, &contagem))
    {
        fprintf(output, "\n");
        return;
    }

    aeroporto_t *aero = gestor_aeroportos_obter_por_codigo(gestor_aeroportos, origem);
    if (!aero)
        fprintf(output, "%s%s%u\n", origem, separador, contagem);
    else
        fprintf(output, "%s%s%s%s%s%s%s%s%u\n",
                aeroporto_obter_codigo(aero), separador,
                aeroporto_obter_nome(aero), separador,
                aeroporto_obter_cidade(aero), separador,
                aeroporto_obter_pais(aero), separador,
                contagem);
}
