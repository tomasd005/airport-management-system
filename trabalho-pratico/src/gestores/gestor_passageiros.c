#include "gestores/gestor_passageiros.h"
#include "parsers/parser.h"
#include "validacoes/validacao_passageiros.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

struct gestor_passageiros
{
    GArray *passageiros;
};

gestor_passageiros_t *gestor_passageiros_criar(void)
{
    gestor_passageiros_t *g = malloc(sizeof(gestor_passageiros_t));
    g->passageiros = g_array_new(FALSE, FALSE, sizeof(passageiro_t *));
    return g;
}

void gestor_passageiros_destruir(gestor_passageiros_t *gestor)
{
    if (!gestor)
        return;
    for (guint i = 0; i < gestor->passageiros->len; i++)
        passageiro_destruir(g_array_index(gestor->passageiros, passageiro_t *, i));
    g_array_free(gestor->passageiros, TRUE);
    free(gestor);
}

void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *p)
{
    if (!gestor || !p)
        return;
    g_array_append_val(gestor->passageiros, p);
}

passageiro_t *gestor_passageiros_obter_por_documento(gestor_passageiros_t *gestor, const char *document_number)
{
    if (!gestor || !document_number)
        return NULL;

    for (guint i = 0; i < gestor->passageiros->len; i++)
    {
        passageiro_t *p = g_array_index(gestor->passageiros, passageiro_t *, i);
        if (strcmp(passageiro_obter_document_number(p), document_number) == 0)
            return p;
    }
    return NULL;
}

unsigned int gestor_passageiros_numero(gestor_passageiros_t *gestor)
{
    return gestor ? gestor->passageiros->len : 0;
}

// Callback interno para o parser
static gboolean adiciona_passageiro_callback(void *contexto, void *objeto)
{
    gestor_passageiros_t *gestor = (gestor_passageiros_t *)contexto;
    passageiro_t *passageiro = (passageiro_t *)objeto;

    if (!gestor || !passageiro)
        return FALSE;

    gestor_passageiros_adicionar(gestor, passageiro);
    return TRUE;
}

void gestor_passageiros_carregar(gestor_passageiros_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    parser_carrega(
        gestor,
        ficheiro_csv,
        adiciona_passageiro_callback,
        (LinhaParaObjeto)valida_passageiro,
        (DestroiObjeto)passageiro_destruir);
}