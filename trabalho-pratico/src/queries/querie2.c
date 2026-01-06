#include "../../include/queries/querie2.h"
#include "../../include/gestores/gestor_avioes.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/entidades/avioes.h"
#include "../../include/entidades/voos.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
    char *id;
    char *fabricante;
    char *modelo;
    guint count;
} ContadorVoos;

static gboolean contem_substring_case_insensitive(const char *texto, const char *busca)
{
    if (!texto || !busca)
        return FALSE;
    if (!*busca)
        return TRUE;

    gchar *texto_lower = g_utf8_strdown(texto, -1);
    gchar *busca_lower = g_utf8_strdown(busca, -1);

    gboolean found = (g_strstr_len(texto_lower, -1, busca_lower) != NULL);

    g_free(texto_lower);
    g_free(busca_lower);

    return found;
}

static gint compara_contadores(gconstpointer a, gconstpointer b, gpointer user_data)
{
    (void)user_data;
    const ContadorVoos *ca = a;
    const ContadorVoos *cb = b;

    if (ca->count > cb->count)
        return -1;
    if (ca->count < cb->count)
        return 1;

    return strcmp(ca->id, cb->id);
}

static void contar_voos_por_aviao(voo_t *voo, void *user_data)
{
    if (!voo || !user_data)
        return;

    const char *status = voo_obter_status(voo);
    if (!status || strcmp(status, "Cancelled") == 0)
        return;

    const char *aircraft_id = voo_obter_aircraft(voo);
    if (!aircraft_id || !*aircraft_id)
        return;

    GHashTable *contagens = (GHashTable *)user_data;

    guint *ptr = g_hash_table_lookup(contagens, aircraft_id);
    if (ptr)
    {
        (*ptr)++;
    }
    else
    {
        guint *novo = g_new(guint, 1);
        *novo = 1;
        g_hash_table_insert(contagens, g_strdup(aircraft_id), novo);
    }
}

static gboolean fabricante_match(const char *fab_aviao, const char *fab_filtro)
{
    if (!fab_filtro || !*fab_filtro)
        return TRUE;

    if (!fab_aviao)
        return FALSE;

    return contem_substring_case_insensitive(fab_aviao, fab_filtro);
}

static void processar_aviao(aviao_t *aviao, gpointer user_data)
{
    if (!aviao || !user_data)
        return;

    gpointer *dados = (gpointer *)user_data;
    GArray *resultados = (GArray *)dados[0];
    GHashTable *contagens = (GHashTable *)dados[1];
    const char *fabricante_filtro = (const char *)dados[2];

    const char *id = aviao_obter_identificador(aviao);
    const char *fabricante = aviao_obter_fabricante(aviao);
    const char *modelo = aviao_obter_modelo(aviao);

    if (!id || !*id)
        return;

    if (!fabricante_match(fabricante, fabricante_filtro))
        return;

    guint *cnt = g_hash_table_lookup(contagens, id);
    if (!cnt || *cnt == 0)
        return;

    ContadorVoos c;
    c.id = g_strdup(id);
    c.fabricante = fabricante ? g_strdup(fabricante) : g_strdup("");
    c.modelo = modelo ? g_strdup(modelo) : g_strdup("");
    c.count = *cnt;

    g_array_append_val(resultados, c);
}

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

void query2(gestor_avioes_t *gestor_avioes, gestor_voos_t *gestor_voos,
            int N, const char *fabricante, const char *comando_completo, FILE *output)
{
    if (!gestor_avioes || !gestor_voos || !output || N <= 0)
    {
        fprintf(output, "\n");
        return;
    }

    int formato_alternativo = usa_formato_alternativo(comando_completo);
    const char *separador = formato_alternativo ? "=" : ";";

    GHashTable *contagens = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, g_free);
    gestor_voos_para_cada(gestor_voos, contar_voos_por_aviao, contagens);

    GArray *resultados = g_array_new(FALSE, FALSE, sizeof(ContadorVoos));
    gpointer dados[3] = {resultados, contagens, (gpointer)fabricante};
    gestor_avioes_para_cada(gestor_avioes, processar_aviao, dados);

    g_array_sort_with_data(resultados, compara_contadores, NULL);

    guint n_imprimir = (guint)N < resultados->len ? (guint)N : resultados->len;

    if (n_imprimir == 0)
    {
        fprintf(output, "\n");
    }
    else
    {
        for (guint i = 0; i < n_imprimir; i++)
        {
            ContadorVoos *c = &g_array_index(resultados, ContadorVoos, i);
            fprintf(output, "%s%s%s%s%s%s%u\n",
                    c->id, separador,
                    c->fabricante, separador,
                    c->modelo, separador,
                    c->count);
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
    g_hash_table_destroy(contagens);
}