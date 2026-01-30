#include "../../include/queries/querie4.h"
#include "../../include/gestores/gestor_reservas.h"
#include "../../include/gestores/gestor_passageiros.h"
#include "../../include/entidades/passageiros.h"
#include "../../include/utils.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/**
 * @brief Verifica se o comando indica uso do formato alternativo.
 *
 * O formato alternativo é indicado quando, após dígitos iniciais e espaços,
 * aparece 'S' ou 's'.
 *
 * @param cmd Comando completo.
 * @return 1 se usar formato alternativo, 0 caso contrário.
 */
static inline int usa_formato_alternativo(const char *cmd)
{
    if (!cmd)
        return 0;
    while (*cmd && isspace(*cmd))
        cmd++;
    while (*cmd && isdigit(*cmd))
        cmd++;
    while (*cmd && isspace(*cmd))
        cmd++;
    return (*cmd == 'S' || *cmd == 's');
}

/**
 * @brief Estrutura auxiliar para contar aparições no top10 de reservas.
 */
typedef struct {
    GHashTable *contador; /**< Hash table para contar aparições de cada passageiro */
} ContadorTop10Ctx;

static inline uint32_t doc_key_de_ptr(gpointer ptr)
{
    return (uint32_t)(GPOINTER_TO_UINT(ptr) - 1);
}

/**
 * @brief Callback para contar passageiros no top10 de cada semana.
 *
 * Incrementa a contagem de cada passageiro presente no top10 de uma semana.
 *
 * @param semana Número da semana (não utilizado aqui).
 * @param top10 Array de strings contendo documentos dos passageiros no top10.
 * @param user_data Ponteiro para ContadorTop10Ctx.
 */
static void contar_semana_top10(int semana, const GPtrArray *top10, void *user_data)
{
    (void)semana;
    ContadorTop10Ctx *ctx = user_data;
    if (!ctx || !top10)
        return;

    for (guint i = 0; i < top10->len; i++) {
        gpointer doc = g_ptr_array_index((GPtrArray *)top10, i);
        guint *c = g_hash_table_lookup(ctx->contador, doc);
        if (c)
            (*c)++;
        else {
            guint *novo = g_new(guint, 1);
            *novo = 1;
            g_hash_table_insert(ctx->contador, doc, novo);
        }
    }
}

/**
 * @brief Executa a Query 4.
 *
 * @param gestor_reservas Gestor de reservas.
 * @param gestor_voos Gestor de voos (não utilizado nesta query).
 * @param gestor_passageiros Gestor de passageiros.
 * @param data_inicio Data de início no formato "YYYY-MM-DD" (opcional).
 * @param data_fim Data de fim no formato "YYYY-MM-DD" (opcional).
 * @param comando_completo Comando completo, usado para definir formato alternativo.
 * @param output Ponteiro para arquivo onde será escrita a saída.
 */
void query4(gestor_reservas_t *gestor_reservas, gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros, const char *data_inicio, const char *data_fim,
            const char *comando_completo, FILE *output)
{
    if (!gestor_reservas || !gestor_passageiros || !output) {
        fprintf(output, "\n");
        return;
    }

    (void)gestor_voos;

    const char *sep = usa_formato_alternativo(comando_completo) ? "=" : ";";

    GHashTable *contador = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);

    if (data_inicio && data_fim) {
        int dia_inicio = utils_parse_date_to_day(data_inicio);
        int dia_fim = utils_parse_date_to_day(data_fim);
        if (dia_inicio < 0 || dia_fim < 0 || dia_inicio > dia_fim) {
            fprintf(output, "\n");
            g_hash_table_destroy(contador);
            return;
        }

        int semana_inicio = utils_week_from_day(dia_inicio);
        int semana_fim = utils_week_from_day(dia_fim);
        for (int semana = semana_inicio; semana <= semana_fim; semana++) {
            const GPtrArray *top10 = gestor_reservas_obter_top10_semana(gestor_reservas, semana);
            if (!top10)
                continue;
            for (guint i = 0; i < top10->len; i++) {
                gpointer doc = g_ptr_array_index((GPtrArray *)top10, i);
                guint *c = g_hash_table_lookup(contador, doc);
                if (c)
                    (*c)++;
                else {
                    guint *novo = g_new(guint, 1);
                    *novo = 1;
                    g_hash_table_insert(contador, doc, novo);
                }
            }
        }
    } else {
        ContadorTop10Ctx ctx = {.contador = contador};
        gestor_reservas_para_cada_top10(gestor_reservas, contar_semana_top10, &ctx);
    }

    if (g_hash_table_size(contador) == 0) {
        fprintf(output, "\n");
        g_hash_table_destroy(contador);
        return;
    }

    uint32_t melhor_doc_key = 0;
    guint melhor_count = 0;
    int tem_melhor = 0;

    GHashTableIter iter;
    gpointer k, v;
    g_hash_table_iter_init(&iter, contador);
    while (g_hash_table_iter_next(&iter, &k, &v)) {
        uint32_t doc_key = doc_key_de_ptr(k);
        guint count = *(guint *)v;
        if (!tem_melhor || count > melhor_count ||
            (count == melhor_count && doc_key < melhor_doc_key)) {
            melhor_doc_key = doc_key;
            melhor_count = count;
            tem_melhor = 1;
        }
    }

    passageiro_t *p =
        tem_melhor ? gestor_passageiros_obter_por_documento_key(gestor_passageiros, melhor_doc_key)
                   : NULL;
    if (p) {
        char doc[10];
        passageiro_formatar_documento(p, doc);
        fprintf(output, "%s%s%s%s%s%s%s%s%s%s%u\n", doc, sep,
                passageiro_obter_primeiro_nome(p) ? passageiro_obter_primeiro_nome(p) : "", sep,
                passageiro_obter_ultimo_nome(p) ? passageiro_obter_ultimo_nome(p) : "", sep,
                passageiro_obter_dob(p) ? passageiro_obter_dob(p) : "", sep,
                passageiro_obter_nacionalidade(p), sep, melhor_count);
    } else
        fprintf(output, "\n");

    g_hash_table_destroy(contador);
}
