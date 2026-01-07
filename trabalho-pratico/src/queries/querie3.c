#include "../../include/queries/querie3.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/gestores/gestor_aeroportos.h"
#include "../../include/entidades/voos.h"
#include "../../include/entidades/aeroportos.h"
#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

typedef struct
{
    char *code;
    guint count;
} ContadorPartidas;

static gint comparar_contadores(gconstpointer a, gconstpointer b)
{
    const ContadorPartidas *ca = a, *cb = b;
    if (ca->count != cb->count)
        return (gint)(cb->count - ca->count);
    return strcmp(ca->code, cb->code);
}

typedef struct
{
    const char *data_inicio;
    const char *data_fim;
    GHashTable *contagens;
} FiltroDatas;

static void contar_voos_validos(voo_t *voo, void *user_data)
{
    if (!voo)
        return;

    FiltroDatas *filtro = user_data;

    if (strcmp(voo_obter_status(voo), "Cancelled") == 0)
        return;

    const char *actual_dep = voo_obter_actual_departure(voo);
    const char *data_partida = (actual_dep && strcmp(actual_dep, "N/A") != 0) ? actual_dep : voo_obter_departure(voo);

    if (!data_partida || strlen(data_partida) < 10)
        return;

    if (strncmp(data_partida, filtro->data_inicio, 10) < 0 || strncmp(data_partida, filtro->data_fim, 10) > 0)
        return;

    const char *origem = voo_obter_origin(voo);
    if (!origem || !*origem)
        return;

    guint *contador = g_hash_table_lookup(filtro->contagens, origem);
    if (contador)
        (*contador)++;
    else
    {
        guint *novo = g_new(guint, 1);
        *novo = 1;
        g_hash_table_insert(filtro->contagens, g_strdup(origem), novo);
    }
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

void query3(gestor_aeroportos_t *gestor_aeroportos,
            gestor_voos_t *gestor_voos,
            const char *data_inicio,
            const char *data_fim,
            const char *comando_completo,
            FILE *output)
{
    if (!gestor_aeroportos || !gestor_voos || !data_inicio || !data_fim || !output)
    {
        fprintf(output, "\n");
        return;
    }

    const char *separador = usa_formato_alternativo(comando_completo) ? "=" : ";";

    GHashTable *contagens = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    FiltroDatas filtro = {.data_inicio = data_inicio, .data_fim = data_fim, .contagens = contagens};

    gestor_voos_para_cada(gestor_voos, contar_voos_validos, &filtro);

    if (g_hash_table_size(contagens) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(contagens);
        return;
    }

    GArray *lista = g_array_new(FALSE, FALSE, sizeof(ContadorPartidas));

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, contagens);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        ContadorPartidas c = {.code = g_strdup(key), .count = *(guint *)value};
        g_array_append_val(lista, c);
    }

    g_array_sort(lista, comparar_contadores);

    ContadorPartidas *melhor = &g_array_index(lista, ContadorPartidas, 0);
    aeroporto_t *aero = gestor_aeroportos_obter_por_codigo(gestor_aeroportos, melhor->code);

    if (!aero)
        fprintf(output, "%s%s%u\n", melhor->code, separador, melhor->count);
    else
        fprintf(output, "%s%s%s%s%s%s%s%s%u\n",
                aeroporto_obter_codigo(aero), separador,
                aeroporto_obter_nome(aero), separador,
                aeroporto_obter_cidade(aero), separador,
                aeroporto_obter_pais(aero), separador,
                melhor->count);

    for (guint i = 0; i < lista->len; i++)
        g_free(g_array_index(lista, ContadorPartidas, i).code);

    g_array_free(lista, TRUE);
    g_hash_table_destroy(contagens);
}
