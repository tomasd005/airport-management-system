#include "gestores/gestor_reservas.h"
#include "gestores/gestor_voos.h"
#include "parsers/parser.h"
#include "validacoes/validacao_reservas.h"
#include "entidades/reservas.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct gestor_reservas
{
    GHashTable *tabela;
    GHashTable *por_passageiro;
};

gestor_reservas_t *gestor_reservas_criar(void)
{
    gestor_reservas_t *g = malloc(sizeof(gestor_reservas_t));
    g->tabela = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)reserva_destruir);
    g->por_passageiro = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_ptr_array_unref);
    return g;
}

void gestor_reservas_destruir(gestor_reservas_t *gestor)
{
    if (!gestor)
        return;
    g_hash_table_destroy(gestor->por_passageiro);
    g_hash_table_destroy(gestor->tabela);
    free(gestor);
}

void gestor_reservas_adicionar(gestor_reservas_t *gestor, reserva_t *reserva)
{
    if (!gestor || !reserva)
        return;

    const char *id = reserva_obter_id(reserva);
    if (!id || g_hash_table_contains(gestor->tabela, id))
    {
        reserva_destruir(reserva);
        return;
    }

    g_hash_table_insert(gestor->tabela, g_strdup(id), reserva);

    const char *doc = reserva_obter_document_number(reserva);
    if (doc)
    {
        GPtrArray *lista = g_hash_table_lookup(gestor->por_passageiro, doc);
        if (!lista)
        {
            lista = g_ptr_array_new();
            g_hash_table_insert(gestor->por_passageiro, (gpointer)doc, lista);
        }
        g_ptr_array_add(lista, reserva);
    }
}

reserva_t *gestor_reservas_obter_por_id(gestor_reservas_t *gestor, const char *id)
{
    if (!gestor || !id)
        return NULL;
    return g_hash_table_lookup(gestor->tabela, id);
}

unsigned gestor_reservas_contar(const gestor_reservas_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->tabela) : 0;
}

unsigned int gestor_reservas_numero(gestor_reservas_t *gestor)
{
    return gestor ? g_hash_table_size(gestor->tabela) : 0;
}

int gestor_reservas_contar_passageiros_voo(gestor_reservas_t *gestor, const char *flight_id)
{
    if (!gestor || !flight_id)
        return 0;

    int count = 0;
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        reserva_t *r = value;
        const char **flight_ids = reserva_obter_flight_ids(r);
        size_t num_voos = reserva_obter_num_voos(r);

        for (size_t i = 0; i < num_voos; i++)
        {
            if (flight_ids[i] && strcmp(flight_ids[i], flight_id) == 0)
            {
                count++;
                break;
            }
        }
    }

    return count;
}

GPtrArray *gestor_reservas_obter_por_passageiro(gestor_reservas_t *gestor, const char *document_number)
{
    if (!gestor || !document_number)
        return NULL;
    return g_hash_table_lookup(gestor->por_passageiro, document_number);
}

void gestor_reservas_para_cada(gestor_reservas_t *gestor, void (*func)(reserva_t *, void *), void *user_data)
{
    if (!gestor || !func)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
        func(value, user_data);
}

static gboolean _adiciona_reserva_callback(void *contexto, void *objeto)
{
    gestor_reservas_adicionar(contexto, objeto);
    return TRUE;
}

static reserva_t *_criar_reserva_de_campos(char **campos)
{
    return campos ? valida_reserva_from_csv(campos) : NULL;
}

void gestor_reservas_carregar(gestor_reservas_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    parser_carrega(gestor, ficheiro_csv, _adiciona_reserva_callback,
                   (LinhaParaObjeto)_criar_reserva_de_campos, (DestroiObjeto)reserva_destruir);
}

void gestor_reservas_carregar_com_validacao(
    gestor_reservas_t *gestor,
    const char *ficheiro_csv,
    gestor_voos_t *gestor_voos,
    gestor_passageiros_t *gestor_passageiros)
{
    if (!gestor || !ficheiro_csv)
        return;

    FILE *file = fopen(ficheiro_csv, "r");
    if (!file)
        return;

    char linha[4096];
    fgets(linha, sizeof(linha), file);

    while (fgets(linha, sizeof(linha), file))
    {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (!linha[0])
            continue;

        char **campos = g_strsplit(linha, ";", -1);
        reserva_t *reserva = valida_reserva_from_csv(campos);

        if (reserva)
        {
            GPtrArray *erros = validar_reserva(reserva, gestor_voos, gestor_passageiros);
            if (!erros || erros->len == 0)
                gestor_reservas_adicionar(gestor, reserva);
            else
                reserva_destruir(reserva);

            if (erros)
                g_ptr_array_free(erros, TRUE);
        }

        g_strfreev(campos);
    }

    fclose(file);
}

static inline int _calcular_semana(const char *data)
{
    int y, m, d;
    if (sscanf(data, "%d-%d-%d", &y, &m, &d) != 3)
        return -1;

    static const int dias_acum[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int dia_ano = dias_acum[m - 1] + d;
    return (dia_ano - 1) / 7 + 1;
}

void gestor_reservas_para_cada_com_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    void (*callback)(int semana, reserva_t *, void *),
    void *user_data)
{
    (void)gestor_voos;
    if (!gestor || !callback)
        return;

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        reserva_t *r = value;
        const char **flight_ids = reserva_obter_flight_ids(r);
        size_t num_voos = reserva_obter_num_voos(r);

        if (num_voos > 0 && flight_ids && flight_ids[0] && strchr(flight_ids[0], '-'))
        {
            int semana = _calcular_semana(flight_ids[0]);
            if (semana >= 0)
                callback(semana, r, user_data);
        }
    }
}

void gestor_reservas_para_cada_semana(
    gestor_reservas_t *gestor,
    gestor_voos_t *gestor_voos,
    const char *data_inicio,
    const char *data_fim,
    void (*callback)(int semana, reserva_t *, void *),
    void *user_data)
{
    (void)gestor_voos;
    if (!gestor || !data_inicio || !data_fim || !callback)
        return;

    int semana_inicio = _calcular_semana(data_inicio);
    int semana_fim = _calcular_semana(data_fim);

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, gestor->tabela);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        reserva_t *r = value;
        const char **flight_ids = reserva_obter_flight_ids(r);
        size_t num_voos = reserva_obter_num_voos(r);

        if (num_voos > 0 && flight_ids && flight_ids[0] && strchr(flight_ids[0], '-'))
        {
            int semana = _calcular_semana(flight_ids[0]);
            if (semana >= semana_inicio && semana <= semana_fim)
                callback(semana, r, user_data);
        }
    }
}