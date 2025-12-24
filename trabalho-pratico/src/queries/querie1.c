#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../../include/gestores/gestor_aeroportos.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/gestores/gestor_reservas.h"
#include "../../include/entidades/aeroportos.h"
#include "../../include/entidades/voos.h"
#include "../../include/queries/querie1.h"

/**
 * @brief Verifica se o comando usa formato alternativo (termina com 'S')
 * @param comando String do comando completo (ex: "1S OPO" ou "1 OPO")
 * @return 1 se usa formato 'S', 0 caso contrário
 */
static int usa_formato_alternativo(const char *comando)
{
    if (!comando)
        return 0;

    // Pula espaços iniciais
    while (*comando && isspace(*comando))
        comando++;

    // Pula os dígitos do número da query
    while (*comando && isdigit(*comando))
        comando++;

    // Verifica se há um 'S' logo após o número
    return (*comando == 'S');
}

/**
 * @brief Conta passageiros que ATERRARAM no aeroporto (destination)
 * @param gestor_voos Gestor de voos
 * @param gestor_reservas Gestor de reservas
 * @param airport_code Código do aeroporto
 * @return Número de passageiros que aterraram (voos não cancelados)
 */
static int conta_passageiros_chegada(gestor_voos_t *gestor_voos,
                                     gestor_reservas_t *gestor_reservas,
                                     const char *airport_code)
{
    if (!gestor_voos || !gestor_reservas || !airport_code)
        return 0;

    int total = 0;

    // Obtém todos os voos com este destination
    GPtrArray *voos = gestor_voos_obter_por_destination(gestor_voos, airport_code);

    if (!voos)
        return 0;

    for (guint i = 0; i < voos->len; i++)
    {
        voo_t *voo = g_ptr_array_index(voos, i);

        // Só conta se NÃO estiver cancelado
        if (strcmp(voo_obter_status(voo), "Cancelled") != 0)
        {
            // Conta quantos passageiros têm reserva neste voo
            total += gestor_reservas_contar_passageiros_voo(gestor_reservas,
                                                            voo_obter_id(voo));
        }
    }

    return total;
}

static int conta_passageiros_partida(gestor_voos_t *gestor_voos,
                                     gestor_reservas_t *gestor_reservas,
                                     const char *airport_code)
{
    if (!gestor_voos || !gestor_reservas || !airport_code)
        return 0;

    int total = 0;

    // Obtém todos os voos com este origin
    GPtrArray *voos = gestor_voos_obter_por_origin(gestor_voos, airport_code);

    if (!voos)
        return 0;

    for (guint i = 0; i < voos->len; i++)
    {
        voo_t *voo = g_ptr_array_index(voos, i);

        // Só conta se NÃO estiver cancelado
        if (strcmp(voo_obter_status(voo), "Cancelled") != 0)
        {
            // Conta quantos passageiros têm reserva neste voo
            total += gestor_reservas_contar_passageiros_voo(gestor_reservas,
                                                            voo_obter_id(voo));
        }
    }

    return total;
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

    // Limpa o código do aeroporto
    char clean_code[16];
    snprintf(clean_code, sizeof(clean_code), "%s", airport_code);
    clean_code[strcspn(clean_code, "\r\n ")] = '\0';

    // Verifica formato de output (com ou sem 'S')
    int formato_alternativo = usa_formato_alternativo(comando_completo);
    const char *separador = formato_alternativo ? "=" : ";";

    // Busca o aeroporto
    aeroporto_t *aeroporto = gestor_aeroportos_obter_por_codigo(gestor_aeroportos, clean_code);

    if (!aeroporto)
    {
        fprintf(output, "\n");
        return;
    }

    // Conta passageiros (NOVO na Fase 2)
    int arrival_count = conta_passageiros_chegada(gestor_voos, gestor_reservas, clean_code);
    int departure_count = conta_passageiros_partida(gestor_voos, gestor_reservas, clean_code);

    // Output com novo formato: code;name;city;country;type;arrival_count;departure_count
    fprintf(output, "%s%s%s%s%s%s%s%s%s%s%d%s%d\n",
            aeroporto_obter_codigo(aeroporto), separador,
            aeroporto_obter_nome(aeroporto), separador,
            aeroporto_obter_cidade(aeroporto), separador,
            aeroporto_obter_pais(aeroporto), separador,
            aeroporto_obter_tipo(aeroporto), separador,
            arrival_count, separador,
            departure_count);
}