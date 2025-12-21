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

// Compara contadores: primeiro por count (desc), depois por ID (asc)
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

    // Se count igual, ordenar por ID crescente
    return strcmp(ca->id, cb->id);
}

// Função para contar voos por avião (apenas voos não cancelados)
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

// Normaliza string para comparação (lowercase, sem espaços extras)
static char *normalizar_string(const char *str)
{
    if (!str)
        return g_strdup("");
    
    char *resultado = g_strdup(str);
    g_strstrip(resultado); // Remove espaços no início/fim
    
    // Converte para lowercase
    for (char *p = resultado; *p; p++)
        *p = g_ascii_tolower(*p);
    
    return resultado;
}

// Verifica se o fabricante do avião corresponde ao filtro
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
    
    gboolean match = (strcmp(fab_aviao_norm, fab_filtro_norm) == 0);
    
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

    // Criar contador
    ContadorVoos c;
    c.id = g_strdup(id);
    c.fabricante = fabricante ? g_strdup(fabricante) : g_strdup("");
    c.modelo = modelo ? g_strdup(modelo) : g_strdup("");

    // Obter contagem de voos (0 se não houver)
    guint *cnt = g_hash_table_lookup(contagens, id);
    c.count = cnt ? *cnt : 0;

    g_array_append_val(resultados, c);
}

void query2(gestor_avioes_t *gestor_avioes, gestor_voos_t *gestor_voos,
            int N, const char *fabricante, FILE *output)
{
    if (!gestor_avioes || !gestor_voos || !output || N <= 0)
    {
        fprintf(output, "\n");
        return;
    }

    // 1. Contar voos não cancelados por avião
    GHashTable *contagens = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    gestor_voos_para_cada(gestor_voos, contar_voos_por_aviao, contagens);

    // 2. Processar todos os aviões (aplicando filtro de fabricante)
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
            fprintf(output, "%s,%s,%s,%u\n", c->id, c->fabricante, c->modelo, c->count);
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