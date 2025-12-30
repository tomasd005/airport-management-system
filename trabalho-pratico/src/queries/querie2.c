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

static char *normalizar_string(const char *str)
{
    if (!str)
        return NULL;

    // Duplicar string
    char *norm = g_strdup(str);
    if (!norm)
        return NULL;

    // Trim espaços (se já tens utils_trim, podes usar)
    char *start = norm;
    while (*start && isspace((unsigned char)*start))
        start++;

    if (start != norm)
        memmove(norm, start, strlen(start) + 1);

    size_t len = strlen(norm);
    while (len > 0 && isspace((unsigned char)norm[len - 1]))
        norm[--len] = '\0';

    // Converter para lowercase
    for (char *p = norm; *p; p++)
        *p = (char)tolower((unsigned char)*p);

    return norm;
}

static gint compara_contadores(gconstpointer a, gconstpointer b, gpointer user_data)
{
    (void)user_data;
    const ContadorVoos *ca = a;
    const ContadorVoos *cb = b;

    // Ordenação por count decrescente
    if (ca->count > cb->count)
        return -1;
    if (ca->count < cb->count)
        return 1;

    // Se count igual, ordenar por ID crescente (LEXICOGRÁFICO)
    return strcmp(ca->id, cb->id);
}

static void contar_voos_por_aviao(const char *key, voo_t *voo, void *user_data)
{
    (void)key;
    if (!voo || !user_data)
        return;

    const char *status = voo_obter_status(voo);
    if (!status || strcmp(status, "Cancelled") == 0)
        return;

    const char *aircraft_id = voo_obter_aircraft(voo);
    if (!aircraft_id || strlen(aircraft_id) == 0)
        return;

    char *aircraft_norm = normalizar_string(aircraft_id);

    GHashTable *contagens = (GHashTable *)user_data;
    guint *ptr = g_hash_table_lookup(contagens, aircraft_norm);
    if (ptr)
    {
        (*ptr)++;
        g_free(aircraft_norm);
    }
    else
    {
        guint *novo = g_new(guint, 1);
        *novo = 1;
        g_hash_table_insert(contagens, aircraft_norm, novo);
    }
}

// CORREÇÃO: Comparação direta SEM normalização
static gboolean fabricante_match(const char *fab_aviao, const char *fab_filtro)
{
    // Se não há filtro, aceita todos
    if (!fab_filtro || strlen(fab_filtro) == 0)
        return TRUE;

    if (!fab_aviao)
        return FALSE;

    // Normalizar ambas as strings para comparação
    char *fab_aviao_norm = normalizar_string(fab_aviao);
    char *fab_filtro_norm = normalizar_string(fab_filtro);

    gboolean match = g_strrstr(fab_aviao_norm, fab_filtro_norm) != NULL;

    g_free(fab_aviao_norm);
    g_free(fab_filtro_norm);

    return match;
}

// Processa cada avião e adiciona à lista de resultados
static void processar_aviao(const char *key, aviao_t *aviao, gpointer user_data)
{
    (void)key;
    if (!aviao || !user_data)
        return;

    gpointer *dados = (gpointer *)user_data;
    GArray *resultados = (GArray *)dados[0];
    GHashTable *contagens = (GHashTable *)dados[1];
    const char *fabricante_filtro = (const char *)dados[2];

    const char *id = aviao_obter_identificador(aviao);
    const char *fabricante = aviao_obter_fabricante(aviao);
    const char *modelo = aviao_obter_modelo(aviao);

    // Validações básicas
    if (!id || strlen(id) == 0)
        return;

    // Aplicar filtro de fabricante
    if (!fabricante_match(fabricante, fabricante_filtro))
        return;

    // Obter contagem de voos (0 se não houver)
    guint *cnt = g_hash_table_lookup(contagens, id);
    guint count = cnt ? *cnt : 0;

    // CORREÇÃO CRÍTICA: Não incluir aviões com 0 voos
    if (count == 0)
        return;

    // Criar contador
    ContadorVoos c;
    c.id = g_strdup(id);
    c.fabricante = fabricante ? g_strdup(fabricante) : g_strdup("");
    c.modelo = modelo ? g_strdup(modelo) : g_strdup("");

    // Obter contagem de voos (0 se não houver)
    char *id_norm = normalizar_string(id);
    guint *cnt_norm = g_hash_table_lookup(contagens, id_norm);
    c.count = cnt_norm ? *cnt_norm : 0;
    g_free(id_norm);

    if (c.count == 0)
    {
        g_free(c.id);
        g_free(c.fabricante);
        g_free(c.modelo);
        return;
    }

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

    // 1. Contar voos não cancelados por avião
    GHashTable *contagens = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    gestor_voos_para_cada(gestor_voos, contar_voos_por_aviao, contagens);

    // 2. Processar todos os aviões (aplicando filtro de fabricante)
    // Só inclui aviões que tenham pelo menos 1 voo
    GArray *resultados = g_array_new(FALSE, FALSE, sizeof(ContadorVoos));
    gpointer dados[3] = {resultados, contagens, (gpointer)fabricante};
    gestor_avioes_para_cada(gestor_avioes, processar_aviao, dados);

    // 3. Ordenar: primeiro por count (desc), depois por ID (asc)
    g_array_sort_with_data(resultados, compara_contadores, NULL);

    // 4. Imprimir top N
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

    // 5. Limpar memória
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