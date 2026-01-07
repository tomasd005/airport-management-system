#include "../../include/queries/querie2.h"
#include "../../include/gestores/gestor_avioes.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/entidades/avioes.h"
#include "../../include/entidades/voos.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct
{
    char *id;
    char *fabricante;
    char *modelo;
    guint count;
} ContadorVoos;

static GHashTable *contagens_cache = NULL;

static void liberar_cache_q2(void)
{
    if (contagens_cache)
    {
        g_hash_table_destroy(contagens_cache);
        contagens_cache = NULL;
    }
}

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

static void contar_voos_por_aviao(voo_t *voo, void *user_data)
{
    if (!voo || strcmp(voo_obter_status(voo), "Cancelled") == 0)
        return;

    const char *aircraft_id = voo_obter_aircraft(voo);
    if (!aircraft_id || !*aircraft_id)
        return;

    GHashTable *contagens = user_data;
    guint *ptr = g_hash_table_lookup(contagens, aircraft_id);

    if (ptr)
        (*ptr)++;
    else
    {
        guint *novo = g_new(guint, 1);
        *novo = 1;
        g_hash_table_insert(contagens, g_strdup(aircraft_id), novo);
    }
}

static void preparar_cache_q2(gestor_voos_t *gestor_voos)
{
    if (contagens_cache)
        return;

    contagens_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    gestor_voos_para_cada(gestor_voos, contar_voos_por_aviao, contagens_cache);
    atexit(liberar_cache_q2);
}

static void processar_aviao(aviao_t *aviao, gpointer user_data)
{
    if (!aviao)
        return;

    gpointer *dados = user_data;
    GArray *resultados = dados[0];
    GHashTable *contagens = dados[1];
    const char *fabricante_filtro = dados[2];

    const char *id = aviao_obter_identificador(aviao);
    const char *fabricante = aviao_obter_fabricante(aviao);

    if (!id || !*id)
        return;

    if (!fabricante_match(fabricante, fabricante_filtro))
        return;

    guint *cnt = g_hash_table_lookup(contagens, id);
    if (!cnt || *cnt == 0)
        return;

    ContadorVoos c = {
        .id = g_strdup(id),
        .fabricante = fabricante ? g_strdup(fabricante) : g_strdup(""),
        .modelo = aviao_obter_modelo(aviao) ? g_strdup(aviao_obter_modelo(aviao)) : g_strdup(""),
        .count = *cnt};
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
    if (!gestor_avioes || !gestor_voos || !output || N <= 0)
    {
        fprintf(output, "\n");
        return;
    }

    const char *separador = usa_formato_alternativo(comando_completo) ? "=" : ";";

    preparar_cache_q2(gestor_voos);

    GArray *resultados = g_array_new(FALSE, FALSE, sizeof(ContadorVoos));
    gpointer dados[3] = {resultados, contagens_cache, (gpointer)fabricante};
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

    for (guint i = 0; i < resultados->len; i++)
    {
        ContadorVoos *c = &g_array_index(resultados, ContadorVoos, i);
        g_free(c->id);
        g_free(c->fabricante);
        g_free(c->modelo);
    }
    g_array_free(resultados, TRUE);
}
