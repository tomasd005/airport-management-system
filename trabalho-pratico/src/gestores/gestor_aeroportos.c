#include "gestores/gestor_aeroportos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_aeroportos.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct gestor_aeroportos
{
    GHashTable *aeroportos;
};

gestor_aeroportos_t *gestor_aeroportos_criar(void)
{
    gestor_aeroportos_t *g = malloc(sizeof(*g));
    g->aeroportos = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        g_free,
        (GDestroyNotify)aeroporto_destruir);
    return g;
}

void gestor_aeroportos_destruir(gestor_aeroportos_t *gestor)
{
    if (!gestor)
        return;
    g_hash_table_destroy(gestor->aeroportos);
    free(gestor);
}

void gestor_aeroportos_adicionar(gestor_aeroportos_t *gestor, aeroporto_t *aeroporto)
{
    if (!gestor || !aeroporto)
        return;
    const char *codigo = aeroporto_obter_codigo(aeroporto);
    g_hash_table_insert(gestor->aeroportos, g_strdup(codigo), aeroporto);
}

aeroporto_t *gestor_aeroportos_obter_por_codigo(gestor_aeroportos_t *gestor, const char *codigo)
{
    if (!gestor || !codigo)
        return NULL;
    return g_hash_table_lookup(gestor->aeroportos, codigo);
}

unsigned int gestor_aeroportos_numero(gestor_aeroportos_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->aeroportos) : 0;
}

unsigned int gestor_aeroportos_total(const gestor_aeroportos_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->aeroportos) : 0;
}

unsigned int gestor_aeroportos_contar(const gestor_aeroportos_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->aeroportos) : 0;
}

// Callback interno para o parser
static gboolean adiciona_aeroporto_callback(void *contexto, gpointer objeto)
{
    gestor_aeroportos_t *gestor = (gestor_aeroportos_t *)contexto;
    aeroporto_t *aeroporto = (aeroporto_t *)objeto;

    if (!gestor || !aeroporto)
        return FALSE;

    gestor_aeroportos_adicionar(gestor, aeroporto);
    return TRUE;
}

void gestor_aeroportos_carregar(gestor_aeroportos_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    parser_carrega(
        gestor,
        ficheiro_csv,
        adiciona_aeroporto_callback,
        (LinhaParaObjeto)valida_aeroporto,
        (DestroiObjeto)aeroporto_destruir);
}