#include "passageiros.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>

struct passageiro
{
    char *document_number;
    char *primeiro_nome;
    char *ultimo_nome;
    char *dob;
    char *nacionalidade;
};

passageiro_t *passageiro_criar(const char *document_number, const char *primeiro_nome,
                               const char *ultimo_nome, const char *dob,
                               const char *nacionalidade, const char *genero,
                               const char *email, const char *telefone,
                               const char *morada, const char *foto)
{
    if (!document_number || !primeiro_nome || !ultimo_nome || !dob || !nacionalidade)
        return NULL;

    passageiro_t *p = malloc(sizeof(passageiro_t));
    if (!p)
        return NULL;

    p->document_number = g_strdup(document_number);
    p->primeiro_nome = g_strdup(primeiro_nome);
    p->ultimo_nome = g_strdup(ultimo_nome);
    p->dob = g_strdup(dob);
    p->nacionalidade = g_strdup(nacionalidade);

    if (!p->document_number || !p->primeiro_nome || !p->ultimo_nome ||
        !p->dob || !p->nacionalidade)
    {
        passageiro_destruir(p);
        return NULL;
    }

    return p;
}

void passageiro_destruir(passageiro_t *p)
{
    if (!p)
        return;

    free(p->document_number);
    free(p->primeiro_nome);
    free(p->ultimo_nome);
    free(p->dob);
    free(p->nacionalidade);
    free(p);
}

const char *passageiro_obter_document_number(const passageiro_t *p)
{
    return p ? p->document_number : NULL;
}

const char *passageiro_obter_primeiro_nome(const passageiro_t *p)
{
    return p ? p->primeiro_nome : NULL;
}

const char *passageiro_obter_ultimo_nome(const passageiro_t *p)
{
    return p ? p->ultimo_nome : NULL;
}

const char *passageiro_obter_dob(const passageiro_t *p)
{
    return p ? p->dob : NULL;
}

const char *passageiro_obter_nacionalidade(const passageiro_t *p)
{
    return p ? p->nacionalidade : NULL;
}

const char *passageiro_obter_genero(const passageiro_t *p)
{
    return p ? "" : NULL;
}

const char *passageiro_obter_email(const passageiro_t *p)
{
    return p ? "" : NULL;
}

const char *passageiro_obter_telefone(const passageiro_t *p)
{
    return p ? "" : NULL;
}

const char *passageiro_obter_morada(const passageiro_t *p)
{
    return p ? "" : NULL;
}

const char *passageiro_obter_foto(const passageiro_t *p)
{
    return p ? "" : NULL;
}