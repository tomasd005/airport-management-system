#include "../../include/queries/querie2.h"
#include "../../include/gestores/gestor_avioes.h"
#include "../../include/entidades/avioes.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct
{
    const char *id;
    const char *fabricante;
    const char *modelo;
    guint count;
} ContadorVoos;

static inline gboolean fabricante_match(const char *fabricante, const char *filtro)
{
    if (!filtro || !*filtro)
        return TRUE;
    if (!fabricante)
        return FALSE;
    return strcmp(fabricante, filtro) == 0;
}

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
