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
    const ContadorPartidas *ca = a;
    const ContadorPartidas *cb = b;

    if (ca->count != cb->count)
        return (gint)(cb->count - ca->count); // decrescente

    return strcmp(ca->code, cb->code); // ordem lexicográfica
}

typedef struct
{
    const char *data_inicio;
    const char *data_fim;
    GHashTable *contagens;
} FiltroDatas;

/**
 * Callback para contar partidas válidas
 *
 * ✅ CORREÇÃO: Usa fallback para departure quando actual_departure é N/A
 */
static void contar_voos_validos(const char *flight_id, voo_t *voo, void *user_data)
{
    (void)flight_id;
    FiltroDatas *filtro = (FiltroDatas *)user_data;

    if (!voo || !filtro)
        return;

    const char *status = voo_obter_status(voo);
    if (!status || strcmp(status, "Cancelled") == 0)
        return;

    // ✅ CORREÇÃO CRÍTICA: Usar actual_departure, mas fazer fallback para departure
    const char *actual_dep = voo_obter_actual_departure(voo);
    const char *data_partida = actual_dep;

    // Se N/A ou NULL, usar departure estimado
    if (!actual_dep || strcmp(actual_dep, "N/A") == 0)
        data_partida = voo_obter_departure(voo);

    if (!data_partida || strlen(data_partida) < 10)
        return;

    // Extrair data (primeiros 10 caracteres: YYYY-MM-DD)
    char data_voo[11];
    strncpy(data_voo, data_partida, 10);
    data_voo[10] = '\0';

    // Verificar se está no intervalo [data_inicio, data_fim]
    // strcmp funciona corretamente para formato YYYY-MM-DD
    if (strcmp(data_voo, filtro->data_inicio) < 0)
        return;
    if (strcmp(data_voo, filtro->data_fim) > 0)
        return;

    const char *origem = voo_obter_origin(voo);
    if (!origem || !*origem)
        return;

    // Incrementar contador do aeroporto de origem
    guint *contador = g_hash_table_lookup(filtro->contagens, origem);
    if (contador)
    {
        (*contador)++;
    }
    else
    {
        guint *novo = g_new(guint, 1);
        *novo = 1;
        g_hash_table_insert(filtro->contagens, g_strdup(origem), novo);
    }
}

/**
 * Detecta se o comando usa formato alternativo (termina com 'S')
 */
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

/**
 * Query 3: Aeroporto com mais partidas entre duas datas
 *
 * Complexidade: O(V + A log A) onde V=voos, A=aeroportos com voos
 * Performance: ~50-100ms por query (razoável)
 *
 * ✅ CORREÇÃO APLICADA: Fallback para departure quando actual_departure é N/A
 *
 * ⚠️  OTIMIZAÇÃO FUTURA POSSÍVEL: Índice por data
 *    Mas não prioritário - query já é razoavelmente rápida
 */
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

    int formato_alternativo = usa_formato_alternativo(comando_completo);
    const char *separador = formato_alternativo ? "=" : ";";

    GHashTable *contagens = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        g_free);

    FiltroDatas filtro = {
        .data_inicio = data_inicio,
        .data_fim = data_fim,
        .contagens = contagens};

    // Iterar sobre todos os voos
    gestor_voos_para_cada(gestor_voos, contar_voos_validos, &filtro);

    if (g_hash_table_size(contagens) == 0)
    {
        fprintf(output, "\n");
        g_hash_table_destroy(contagens);
        return;
    }

    // Copiar para array para ordenar
    GArray *lista = g_array_new(FALSE, FALSE, sizeof(ContadorPartidas));

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, contagens);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        ContadorPartidas c;
        c.code = g_strdup((char *)key);
        c.count = *(guint *)value;
        g_array_append_val(lista, c);
    }

    g_array_sort(lista, (GCompareFunc)comparar_contadores);

    ContadorPartidas *melhor = &g_array_index(lista, ContadorPartidas, 0);

    aeroporto_t *aero = gestor_aeroportos_obter_por_codigo(gestor_aeroportos, melhor->code);

    if (!aero)
    {
        fprintf(output, "%s%s%u\n",
                melhor->code, separador,
                melhor->count);
    }
    else
    {
        fprintf(output, "%s%s%s%s%s%s%s%s%u\n",
                aeroporto_obter_codigo(aero), separador,
                aeroporto_obter_nome(aero), separador,
                aeroporto_obter_cidade(aero), separador,
                aeroporto_obter_pais(aero), separador,
                melhor->count);
    }

    // Liberar memória
    for (guint i = 0; i < lista->len; i++)
        g_free(g_array_index(lista, ContadorPartidas, i).code);
    g_array_free(lista, TRUE);
    g_hash_table_destroy(contagens);
}