#include "../../include/queries/querie5.h"
#include <glib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static inline int usa_formato_alternativo(const char *cmd)
{
    while (*cmd && isspace(*cmd))
        cmd++;
    while (*cmd && isdigit(*cmd))
        cmd++;
    return (*cmd == 'S');
}

typedef struct
{
    char *airline;
    guint count;
    double avg_delay;
} ResultadoQ5;

static gint cmp_q5(gconstpointer a, gconstpointer b)
{
    const ResultadoQ5 *ra = a, *rb = b;

    double diff = ra->avg_delay - rb->avg_delay;
    if (diff > 1e-9)
        return -1;
    if (diff < -1e-9)
        return 1;

    return strcmp(ra->airline, rb->airline);
}

typedef struct
{
    GArray *res;
} Q5Contexto;

static void adicionar_resultado_q5(const char *airline, guint count, double total_delay, void *user_data)
{
    Q5Contexto *ctx = user_data;
    if (!ctx || !airline || !*airline || count == 0)
        return;

    ResultadoQ5 r = {
        .airline = g_strdup(airline),
        .count = count,
        .avg_delay = total_delay / count};
    g_array_append_val(ctx->res, r);
}

void query5(gestor_voos_t *gestor_voos, int N, const char *comando_completo, FILE *output)
{
    if (!gestor_voos || !output || N <= 0)
    {
        fprintf(output, "\n");
        return;
    }

    const char *sep = usa_formato_alternativo(comando_completo) ? "=" : ";";

    GArray *res = g_array_new(FALSE, FALSE, sizeof(ResultadoQ5));
    Q5Contexto ctx = {.res = res};
    gestor_voos_para_cada_atraso(gestor_voos, adicionar_resultado_q5, &ctx);

    if (res->len == 0)
    {
        fprintf(output, "\n");
        g_array_free(res, TRUE);
        return;
    }

    g_array_sort(res, cmp_q5);

    guint limite = res->len < (guint)N ? res->len : (guint)N;
    for (guint i = 0; i < limite; i++)
    {
        ResultadoQ5 *r = &g_array_index(res, ResultadoQ5, i);
        fprintf(output, "%s%s%u%s%.3f\n", r->airline, sep, r->count, sep, r->avg_delay);
    }

    for (guint i = 0; i < res->len; i++)
        g_free(g_array_index(res, ResultadoQ5, i).airline);

    g_array_free(res, TRUE);
}
