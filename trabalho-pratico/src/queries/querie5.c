#include "../../include/queries/querie5.h"
#include "../../include/entidades/voos.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>

static int usa_formato_alternativo(const char *cmd)
{
    while (*cmd && isspace(*cmd))
        cmd++;
    while (*cmd && isdigit(*cmd))
        cmd++;
    return (*cmd == 'S');
}

typedef struct
{
    guint count;
    double total_delay;
} InfoCompanhia;

typedef struct
{
    char *airline;
    guint count;
    double avg_delay;
} ResultadoQ5;

static void acumular_atraso(voo_t *voo, void *user_data)
{
    GHashTable *mapa = user_data;

    // Verificar se o voo tem status "Delayed"
    const char *status = voo_obter_status(voo);
    if (!status || strcmp(status, "Delayed") != 0)
        return;

    const char *airline = voo_obter_airline(voo);
    if (!airline || airline[0] == '\0')
        return;

    // CORREÇÃO CRÍTICA: calcular atraso corretamente
    double atraso = voo_calcular_atraso_minutos(voo);

    // Aceitar apenas atrasos > 0 (voos com atraso positivo)
    // NOTA: Se atraso == 0, tecnicamente não há atraso
    if (atraso <= 0.0)
        return;

    InfoCompanhia *info = g_hash_table_lookup(mapa, airline);

    if (info)
    {
        info->count++;
        info->total_delay += atraso;
    }
    else
    {
        InfoCompanhia *novo = g_new(InfoCompanhia, 1);
        novo->count = 1;
        novo->total_delay = atraso;

        g_hash_table_insert(mapa, g_strdup(airline), novo);
    }
}

static gint cmp_q5(gconstpointer a, gconstpointer b)
{
    const ResultadoQ5 *ra = a;
    const ResultadoQ5 *rb = b;

    // Comparação de doubles: diferença maior que EPSILON
    const double EPSILON = 1e-9;
    double diff = ra->avg_delay - rb->avg_delay;

    if (diff > EPSILON)
        return -1; // ra > rb → ordem decrescente (maior primeiro)
    if (diff < -EPSILON)
        return 1; // ra < rb

    // Empate: ordenar alfabeticamente
    return strcmp(ra->airline, rb->airline);
}

void query5(
    gestor_voos_t *gestor_voos,
    int N,
    const char *comando_completo,
    FILE *output)
{
    if (!gestor_voos || !output || N <= 0)
    {
        fprintf(output, "\n");
        return;
    }

    const char *sep = usa_formato_alternativo(comando_completo) ? "=" : ";";

    GHashTable *mapa = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, g_free);

    // Itera apenas sobre voos com status "Delayed"
    gestor_voos_para_cada_atrasado(
        gestor_voos,
        acumular_atraso,
        mapa);

    if (g_hash_table_size(mapa) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(mapa);
        return;
    }

    GArray *res = g_array_new(FALSE, FALSE, sizeof(ResultadoQ5));

    GHashTableIter it;
    gpointer k, v;
    g_hash_table_iter_init(&it, mapa);

    while (g_hash_table_iter_next(&it, &k, &v))
    {
        InfoCompanhia *info = v;

        ResultadoQ5 r;
        r.airline = g_strdup(k);
        r.count = info->count;
        r.avg_delay = info->total_delay / info->count;

        g_array_append_val(res, r);
    }

    g_array_sort(res, (GCompareFunc)cmp_q5);

    guint limite = res->len < (guint)N ? res->len : (guint)N;

    for (guint i = 0; i < limite; i++)
    {
        ResultadoQ5 *r = &g_array_index(res, ResultadoQ5, i);

        fprintf(output, "%s%s%u%s%.3f\n",
                r->airline, sep,
                r->count, sep,
                r->avg_delay);
    }

    for (guint i = 0; i < res->len; i++)
        g_free(g_array_index(res, ResultadoQ5, i).airline);

    g_array_free(res, TRUE);
    g_hash_table_destroy(mapa);
}