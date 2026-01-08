#include "gestores/gestor_reservas.h"
#include "gestores/gestor_passageiros.h"
#include "gestores/gestor_voos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_reservas.h"
#include "entidades/passageiros.h"
#include "entidades/voos.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct
{
    const char *destino;
    guint count;
} melhor_destino_t;

typedef struct
{
    const char *doc;
    double total;
} gasto_t;

#define MAX_COLUNAS_RESERVAS 16
#define RESERVA_COLS 8

struct gestor_reservas
{
    guint total_reservas;
    GHashTable *gastos_por_semana;
    GHashTable *top10_por_semana;
    GHashTable *destinos_por_nacionalidade;
    GHashTable *melhor_dest_por_nacionalidade;
};

static GHashTable *criar_mapa_gastos(void)
{
    return g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
}

static GHashTable *criar_mapa_destinos(void)
{
    return g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
}

gestor_reservas_t *gestor_reservas_criar(void)
{
    gestor_reservas_t *g = malloc(sizeof(*g));
    g->total_reservas = 0;
    g->gastos_por_semana = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)g_hash_table_destroy);
    g->top10_por_semana = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)g_ptr_array_unref);
    g->destinos_por_nacionalidade = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_hash_table_destroy);
    g->melhor_dest_por_nacionalidade = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
    return g;
}

void gestor_reservas_destruir(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;

    if (gestor->gastos_por_semana)
        g_hash_table_destroy(gestor->gastos_por_semana);
    if (gestor->top10_por_semana)
        g_hash_table_destroy(gestor->top10_por_semana);
    if (gestor->destinos_por_nacionalidade)
        g_hash_table_destroy(gestor->destinos_por_nacionalidade);
    if (gestor->melhor_dest_por_nacionalidade)
        g_hash_table_destroy(gestor->melhor_dest_por_nacionalidade);
    free(gestor);
}

unsigned int gestor_reservas_numero(gestor_reservas_t *gestor)
{
    return gestor ? gestor->total_reservas : 0;
}

static void acumular_gastos_semana(gestor_reservas_t *gestor,
                                   const char *doc,
                                   double preco,
                                   const char **flight_ids,
                                   size_t num_voos,
                                   gestor_voos_t *gestor_voos,
                                   gestor_passageiros_t *gestor_passageiros)
{
    if (!gestor || !doc || !flight_ids || num_voos == 0 || !gestor_voos || !gestor_passageiros)
        return;

    voo_t *voo = gestor_voos_obter_por_id(gestor_voos, flight_ids[0]);
    if (!voo)
        return;

    int semana = voo_obter_semana(voo);
    if (semana < 0)
        return;

    GHashTable *gastos = g_hash_table_lookup(gestor->gastos_por_semana, GINT_TO_POINTER(semana));
    if (!gastos)
    {
        gastos = criar_mapa_gastos();
        g_hash_table_insert(gestor->gastos_por_semana, GINT_TO_POINTER(semana), gastos);
    }

    passageiro_t *p = doc ? gestor_passageiros_obter_por_documento(gestor_passageiros, doc) : NULL;
    const char *doc_estavel = p ? passageiro_obter_document_number(p) : NULL;
    if (!doc_estavel)
        return;

    double *total = g_hash_table_lookup(gastos, doc_estavel);
    if (total)
        *total += preco;
    else
    {
        double *novo = g_new(double, 1);
        *novo = preco;
        g_hash_table_insert(gastos, (gpointer)doc_estavel, novo);
    }
}

static void acumular_destinos_nacionalidade(gestor_reservas_t *gestor,
                                            const char *doc,
                                            const char **flight_ids,
                                            size_t num_voos,
                                            gestor_voos_t *gestor_voos,
                                            gestor_passageiros_t *gestor_passageiros)
{
    if (!gestor || !doc || !flight_ids || num_voos == 0 || !gestor_voos || !gestor_passageiros)
        return;

    passageiro_t *p = gestor_passageiros_obter_por_documento(gestor_passageiros, doc);
    if (!p)
        return;

    const char *nac = passageiro_obter_nacionalidade(p);
    if (!nac || !*nac)
        return;

    GHashTable *destinos = g_hash_table_lookup(gestor->destinos_por_nacionalidade, nac);
    if (!destinos)
    {
        destinos = criar_mapa_destinos();
        g_hash_table_insert(gestor->destinos_por_nacionalidade, (gpointer)nac, destinos);
    }

    for (size_t i = 0; i < num_voos; i++)
    {
        if (!flight_ids[i])
            continue;

        voo_t *voo = gestor_voos_obter_por_id(gestor_voos, flight_ids[i]);
        if (!voo)
            continue;

        if (voo_obter_status_codigo(voo) == 2)
            continue;

        const char *dest = voo_obter_destination(voo);
        if (!dest || !*dest)
            continue;

        guint *count = g_hash_table_lookup(destinos, dest);
        if (count)
            (*count)++;
        else
        {
            guint *novo = g_new(guint, 1);
            *novo = 1;
            g_hash_table_insert(destinos, (gpointer)dest, novo);
        }
    }
}

static void acumular_passageiros_voos(gestor_voos_t *gestor_voos, const char **flight_ids, size_t num_voos)
{
    if (!gestor_voos || !flight_ids || num_voos == 0)
        return;

    for (size_t i = 0; i < num_voos; i++)
    {
        if (!flight_ids[i])
            continue;
        voo_t *voo = gestor_voos_obter_por_id(gestor_voos, flight_ids[i]);
        if (!voo)
            continue;
        voo_incrementar_passageiros(voo, 1);
    }
}

static gboolean reserva_valida_logica_view(const char **flight_ids,
                                           size_t num_voos,
                                           const char *document_number,
                                           gestor_voos_t *gestor_voos,
                                           gestor_passageiros_t *gestor_passageiros)
{
    if (!flight_ids || num_voos == 0)
        return FALSE;

    voo_t *voo1 = NULL;
    voo_t *voo2 = NULL;

    if (gestor_voos)
    {
        for (size_t i = 0; i < num_voos; i++)
        {
            const char *fid = flight_ids[i];
            if (!fid)
                return FALSE;

            voo_t *voo = gestor_voos_obter_por_id(gestor_voos, fid);
            if (!voo)
                return FALSE;

            if (i == 0)
                voo1 = voo;
            else if (i == 1)
                voo2 = voo;
        }
    }

    if (gestor_passageiros && document_number)
    {
        if (!gestor_passageiros_obter_por_documento(gestor_passageiros, document_number))
            return FALSE;
    }

    if (num_voos == 2 && gestor_voos && voo1 && voo2)
    {
        const char *dest1 = voo_obter_destination(voo1);
        const char *orig2 = voo_obter_origin(voo2);
        if (!dest1 || !orig2 || strcmp(dest1, orig2) != 0)
            return FALSE;
    }

    return TRUE;
}

void gestor_reservas_carregar_com_validacao(
    gestor_reservas_t *gestor,
    const char *ficheiro_csv,
    gestor_voos_t *gestor_voos,
    gestor_passageiros_t *gestor_passageiros)
{
    if (!gestor || !ficheiro_csv)
        return;

    FILE *ficheiro = fopen(ficheiro_csv, "r");
    if (!ficheiro)
    {
        perror("Erro ao abrir ficheiro CSV");
        return;
    }
    setvbuf(ficheiro, NULL, _IOFBF, 1 << 20);

    char *nome_base = utils_obtem_nome_ficheiro(ficheiro_csv);
    char *caminho_erros = g_strdup_printf("resultados/%s_errors.csv", nome_base);
    g_free(nome_base);

    FILE *ficheiro_erros = fopen(caminho_erros, "w");
    g_free(caminho_erros);
    if (ficheiro_erros)
        setvbuf(ficheiro_erros, NULL, _IOFBF, 1 << 20);

    char *linha = NULL;
    size_t tamanho = 0;
    char *linha_parse = NULL;
    size_t tamanho_parse = 0;
    ssize_t lidos;

    if ((lidos = getline(&linha, &tamanho, ficheiro)) != -1)
    {
        if (ficheiro_erros)
            fprintf(ficheiro_erros, "%s", linha);
    }

    while ((lidos = getline(&linha, &tamanho, ficheiro)) != -1)
    {
        size_t necessario = (size_t)lidos + 1;
        if (necessario > tamanho_parse)
        {
            char *novo = realloc(linha_parse, necessario);
            if (!novo)
                break;
            linha_parse = novo;
            tamanho_parse = necessario;
        }
        memcpy(linha_parse, linha, necessario);

        char *colunas[MAX_COLUNAS_RESERVAS + 1];
        int numColunas = parser_dividir_csv(linha_parse, colunas, MAX_COLUNAS_RESERVAS);
        if (numColunas < RESERVA_COLS)
        {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%s", linha);
            continue;
        }

        const char *flight_ids[2] = {0};
        size_t num_voos = 0;
        const char *document_number = NULL;
        double preco = 0.0;
        if (!valida_reserva_campos(colunas, flight_ids, &num_voos, &document_number, &preco) ||
            !reserva_valida_logica_view(flight_ids, num_voos, document_number, gestor_voos, gestor_passageiros))
        {
            if (ficheiro_erros)
                fprintf(ficheiro_erros, "%s", linha);
            continue;
        }

        gestor->total_reservas++;
        acumular_passageiros_voos(gestor_voos, flight_ids, num_voos);
        acumular_gastos_semana(gestor, document_number, preco, flight_ids, num_voos,
                               gestor_voos, gestor_passageiros);
        acumular_destinos_nacionalidade(gestor, document_number, flight_ids, num_voos,
                                        gestor_voos, gestor_passageiros);
    }

    free(linha_parse);
    free(linha);
    fclose(ficheiro);
    if (ficheiro_erros)
        fclose(ficheiro_erros);
}

static gint cmp_gastos(gconstpointer a, gconstpointer b)
{
    const gasto_t *ga = a;
    const gasto_t *gb = b;
    if (ga->total > gb->total)
        return -1;
    if (ga->total < gb->total)
        return 1;
    return strcmp(ga->doc, gb->doc);
}

static void top10_inserir_ordenado(GArray *top, const gasto_t *novo)
{
    guint pos = 0;
    while (pos < top->len)
    {
        gasto_t *cur = &g_array_index(top, gasto_t, pos);
        if (cmp_gastos(novo, cur) < 0)
            break;
        pos++;
    }

    guint len = top->len;
    g_array_set_size(top, len + 1);
    for (guint i = len; i > pos; i--)
        g_array_index(top, gasto_t, i) = g_array_index(top, gasto_t, i - 1);
    g_array_index(top, gasto_t, pos) = *novo;
}

void gestor_reservas_finalizar(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;

    if (gestor->gastos_por_semana)
    {
        GHashTableIter iter_sem;
        gpointer skey, sval;
        g_hash_table_iter_init(&iter_sem, gestor->gastos_por_semana);

        while (g_hash_table_iter_next(&iter_sem, &skey, &sval))
        {
            int semana = GPOINTER_TO_INT(skey);
            GHashTable *gastos = (GHashTable *)sval;
            GArray *top = g_array_sized_new(FALSE, FALSE, sizeof(gasto_t), 10);

            GHashTableIter iter_g;
            gpointer k, v;
            g_hash_table_iter_init(&iter_g, gastos);

            while (g_hash_table_iter_next(&iter_g, &k, &v))
            {
                gasto_t g = {.doc = (const char *)k, .total = *(double *)v};
                if (top->len < 10)
                {
                    top10_inserir_ordenado(top, &g);
                }
                else
                {
                    gasto_t *pior = &g_array_index(top, gasto_t, top->len - 1);
                    if (cmp_gastos(&g, pior) < 0)
                    {
                        top10_inserir_ordenado(top, &g);
                        g_array_set_size(top, 10);
                    }
                }
            }

            GPtrArray *top10 = g_ptr_array_new();
            for (guint i = 0; i < top->len; i++)
            {
                gasto_t *g = &g_array_index(top, gasto_t, i);
                g_ptr_array_add(top10, (gpointer)g->doc);
            }

            g_hash_table_insert(gestor->top10_por_semana, GINT_TO_POINTER(semana), top10);
            g_array_free(top, TRUE);
        }

        g_hash_table_destroy(gestor->gastos_por_semana);
        gestor->gastos_por_semana = NULL;
    }

    if (gestor->destinos_por_nacionalidade)
    {
        GHashTableIter iter_nac;
        gpointer nk, nv;
        g_hash_table_iter_init(&iter_nac, gestor->destinos_por_nacionalidade);

        while (g_hash_table_iter_next(&iter_nac, &nk, &nv))
        {
            const char *nac = nk;
            GHashTable *destinos = nv;

            const char *melhor = NULL;
            guint melhor_count = 0;

            GHashTableIter iter_d;
            gpointer dk, dv;
            g_hash_table_iter_init(&iter_d, destinos);

            while (g_hash_table_iter_next(&iter_d, &dk, &dv))
            {
                const char *dest = dk;
                guint count = *(guint *)dv;
                if (count > melhor_count || (count == melhor_count && (!melhor || strcmp(dest, melhor) < 0)))
                {
                    melhor = dest;
                    melhor_count = count;
                }
            }

            if (melhor)
            {
                melhor_destino_t *md = g_new0(melhor_destino_t, 1);
                md->destino = melhor;
                md->count = melhor_count;
                g_hash_table_insert(gestor->melhor_dest_por_nacionalidade, (gpointer)nac, md);
            }
        }

        g_hash_table_destroy(gestor->destinos_por_nacionalidade);
        gestor->destinos_por_nacionalidade = NULL;
    }
}

const GPtrArray *gestor_reservas_obter_top10_semana(gestor_reservas_t *gestor, int semana)
{
    if (!gestor || !gestor->top10_por_semana)
        return NULL;
    return g_hash_table_lookup(gestor->top10_por_semana, GINT_TO_POINTER(semana));
}

gboolean gestor_reservas_obter_melhor_destino_nacionalidade(gestor_reservas_t *gestor, const char *nac, const char **destino, guint *count)
{
    if (!gestor || !nac || !destino || !count)
        return FALSE;

    melhor_destino_t *md = g_hash_table_lookup(gestor->melhor_dest_por_nacionalidade, nac);
    if (!md || !md->destino)
        return FALSE;

    *destino = md->destino;
    *count = md->count;
    return TRUE;
}

void gestor_reservas_para_cada_top10(gestor_reservas_t *gestor, void (*callback)(int semana, const GPtrArray *top10, void *user_data), void *user_data)
{
    if (!gestor || !callback || !gestor->top10_por_semana)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->top10_por_semana);

    while (g_hash_table_iter_next(&iter, &key, &value))
        callback(GPOINTER_TO_INT(key), value, user_data);
}
