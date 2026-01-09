#include "../../include/queries/querie2.h"
#include "../../include/gestores/gestor_avioes.h"
#include "../../include/entidades/avioes.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * @brief Estrutura auxiliar para armazenar dados do avião e contagem de voos.
 */
typedef struct
{
    const char *id;        
    const char *fabricante;
    const char *modelo;    
    guint count;           
} ContadorVoos;

/**
 * @brief Verifica se o avião corresponde ao filtro de fabricante.
 *
 * @param fabricante Nome do fabricante do avião.
 * @param filtro Nome do fabricante a filtrar. NULL ou string vazia significa sem filtro.
 * @return TRUE se corresponder ou não houver filtro, FALSE caso contrário.
 */
static inline gboolean fabricante_match(const char *fabricante, const char *filtro)
{
    if (!filtro || !*filtro)
        return TRUE;
    if (!fabricante)
        return FALSE;
    return strcmp(fabricante, filtro) == 0;
}

/**
 * @brief Função de comparação para ordenar os contadores de voos.
 *
 * Ordena em ordem decrescente pelo número de voos. Em caso de empate,
 * ordena pelo identificador do avião em ordem alfabética.
 *
 * @param a Primeiro elemento.
 * @param b Segundo elemento.
 * @param user_data Não usado.
 * @return Valor <0, 0 ou >0 conforme comparação.
 */
static gint compara_contadores(gconstpointer a, gconstpointer b, gpointer user_data)
{
    (void)user_data;
    const ContadorVoos *ca = a, *cb = b;

    if (ca->count > cb->count)
        return -1;
    if (ca->count < cb->count)
        return 1;
    return strcmp(ca->id, cb->id);
}

/**
 * @brief Processa cada avião do gestor, adicionando à lista de resultados se válido.
 *
 * @param aviao Ponteiro para o avião.
 * @param user_data Array de ponteiros com: [0]=GArray resultados, [2]=filtro de fabricante.
 */
static void processar_aviao(aviao_t *aviao, gpointer user_data)
{
    if (!aviao)
        return;

    gpointer *dados = user_data;
    GArray *resultados = dados[0];
    const char *fabricante_filtro = dados[2];

    const char *id = aviao_obter_identificador(aviao);
    const char *fabricante = aviao_obter_fabricante(aviao);

    if (!id || !*id)
        return;

    if (!fabricante_match(fabricante, fabricante_filtro))
        return;

    int cnt = aviao_obter_contagem_voos(aviao);
    if (cnt <= 0)
        return;

    const char *modelo = aviao_obter_modelo(aviao);
    ContadorVoos c = {
        .id = id,
        .fabricante = fabricante ? fabricante : "",
        .modelo = modelo ? modelo : "",
        .count = (guint)cnt};
    g_array_append_val(resultados, c);
}

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

    GArray *resultados = g_array_new(FALSE, FALSE, sizeof(ContadorVoos));
    gpointer dados[3] = {resultados, NULL, (gpointer)fabricante};
    gestor_avioes_para_cada(gestor_avioes, processar_aviao, dados);

    g_array_sort_with_data(resultados, compara_contadores, NULL);

    guint n_imprimir = (guint)N < resultados->len ? (guint)N : resultados->len;

    if (n_imprimir == 0)
        fprintf(output, "\n");
    else
    {
        for (guint i = 0; i < n_imprimir; i++)
        {
            ContadorVoos *c = &g_array_index(resultados, ContadorVoos, i);
            fprintf(output, "%s%s%s%s%s%s%u\n", c->id, separador, c->fabricante, separador, c->modelo, separador, c->count);
        }
    }

    g_array_free(resultados, TRUE);
}
