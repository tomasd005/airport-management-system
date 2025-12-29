#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../../include/gestores/gestor_aeroportos.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/gestores/gestor_reservas.h"
#include "../../include/entidades/aeroportos.h"
#include "../../include/entidades/voos.h"
#include "../../include/queries/querie1.h"

static int usa_formato_alternativo(const char *comando)
{
    if (!comando)
        return 0;

    while (*comando && isspace(*comando))
        comando++;
    while (*comando && isdigit(*comando))
        comando++;

    return (*comando == 'S');
}

/* Contexto para contagem de passageiros */
typedef struct
{
    gestor_reservas_t *gestor_reservas;
    int total;
} ContextoContagem;

/* Callback para contar passageiros em voos de chegada */
static void contar_passageiros_voo(voo_t *voo, void *user_data)
{
    ContextoContagem *ctx = user_data;

    // Só conta se NÃO estiver cancelado
    if (strcmp(voo_obter_status(voo), "Cancelled") != 0)
    {
        ctx->total += gestor_reservas_contar_passageiros_voo(
            ctx->gestor_reservas,
            voo_obter_id(voo));
    }
}

/* Conta passageiros que ATERRARAM no aeroporto (destination) */
static int conta_passageiros_chegada(
    gestor_voos_t *gestor_voos,
    gestor_reservas_t *gestor_reservas,
    const char *airport_code)
{
    if (!gestor_voos || !gestor_reservas || !airport_code)
        return 0;

    ContextoContagem ctx = {
        .gestor_reservas = gestor_reservas,
        .total = 0};

    // USA ITERADOR em vez de obter GPtrArray diretamente
    gestor_voos_para_cada_destino(gestor_voos, airport_code,
                                  contar_passageiros_voo, &ctx);

    return ctx.total;
}

/* Conta passageiros que PARTIRAM do aeroporto (origin) */
static int conta_passageiros_partida(
    gestor_voos_t *gestor_voos,
    gestor_reservas_t *gestor_reservas,
    const char *airport_code)
{
    if (!gestor_voos || !gestor_reservas || !airport_code)
        return 0;

    ContextoContagem ctx = {
        .gestor_reservas = gestor_reservas,
        .total = 0};

    // USA ITERADOR em vez de obter GPtrArray diretamente
    gestor_voos_para_cada_origem(gestor_voos, airport_code,
                                 contar_passageiros_voo, &ctx);

    return ctx.total;
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

    int formato_alternativo = usa_formato_alternativo(comando_completo);
    const char *separador = formato_alternativo ? "=" : ";";

    aeroporto_t *aeroporto = gestor_aeroportos_obter_por_codigo(
        gestor_aeroportos, clean_code);

    if (!aeroporto)
    {
        fprintf(output, "\n");
        return;
    }

    // Conta passageiros usando funções encapsuladas
    int arrival_count = conta_passageiros_chegada(
        gestor_voos, gestor_reservas, clean_code);
    int departure_count = conta_passageiros_partida(
        gestor_voos, gestor_reservas, clean_code);

    fprintf(output, "%s%s%s%s%s%s%s%s%s%s%d%s%d\n",
            aeroporto_obter_codigo(aeroporto), separador,
            aeroporto_obter_nome(aeroporto), separador,
            aeroporto_obter_cidade(aeroporto), separador,
            aeroporto_obter_pais(aeroporto), separador,
            aeroporto_obter_tipo(aeroporto), separador,
            arrival_count, separador,
            departure_count);
}