#include "passageiros.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

/**
 * @struct passageiro
 * @brief Representa um passageiro com dados pessoais.
 */
struct passageiro
{
    uint32_t doc_key;
    char *primeiro_nome;
    char *ultimo_nome;
    char *dob;
    const char *nacionalidade;
    uint32_t *destinos_counts;
    unsigned char owns_strings;
    unsigned char has_details;
};

static GHashTable *g_nacionalidades_intern = NULL;
static pthread_mutex_t g_nacionalidades_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char *internar_nacionalidade(const char *nac)
{
    if (!nac || !*nac)
        return "";

    pthread_mutex_lock(&g_nacionalidades_mutex);
    if (!g_nacionalidades_intern)
        g_nacionalidades_intern = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    gpointer encontrado = g_hash_table_lookup(g_nacionalidades_intern, nac);
    if (encontrado) {
        pthread_mutex_unlock(&g_nacionalidades_mutex);
        return (const char *)encontrado;
    }

    char *dup = g_strdup(nac);
    g_hash_table_insert(g_nacionalidades_intern, dup, dup);
    pthread_mutex_unlock(&g_nacionalidades_mutex);
    return dup;
}

/**
 * @brief Cria um novo passageiro com os dados fornecidos.
 *
 * @param document_number Número do documento 
 * @param primeiro_nome Primeiro nome 
 * @param ultimo_nome Último nome 
 * @param dob Data de nascimento 
 * @param nacionalidade Nacionalidade 
 * @param genero Gênero 
 * @param email Email 
 * @param telefone Telefone 
 * @param morada Morada 
 * @param foto Foto 
 * @return Ponteiro para passageiro_t recém-criado, ou NULL em caso de erro
 */
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

    if (!utils_document_number_key(document_number, &p->doc_key))
    {
        free(p);
        return NULL;
    }

    p->primeiro_nome = g_strdup(primeiro_nome);
    p->ultimo_nome = g_strdup(ultimo_nome);
    p->dob = g_strdup(dob);
    p->nacionalidade = internar_nacionalidade(nacionalidade);
    p->destinos_counts = NULL;
    p->owns_strings = 1;
    p->has_details = 1;

    if (!p->primeiro_nome || !p->ultimo_nome || !p->dob || !p->nacionalidade)
    {
        passageiro_destruir(p);
        return NULL;
    }

    return p;
}

passageiro_t *passageiro_criar_compacto(const char *document_number, const char *nacionalidade)
{
    if (!document_number || !nacionalidade)
        return NULL;

    passageiro_t *p = malloc(sizeof(passageiro_t));
    if (!p)
        return NULL;

    if (!utils_document_number_key(document_number, &p->doc_key))
    {
        free(p);
        return NULL;
    }

    p->primeiro_nome = NULL;
    p->ultimo_nome = NULL;
    p->dob = NULL;
    p->nacionalidade = internar_nacionalidade(nacionalidade);
    p->destinos_counts = NULL;
    p->owns_strings = 0;
    p->has_details = 0;
    return p;
}

passageiro_t *passageiro_criar_borrowed(const char *document_number, const char *primeiro_nome,
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

    if (!utils_document_number_key(document_number, &p->doc_key))
    {
        free(p);
        return NULL;
    }

    p->primeiro_nome = (char *)primeiro_nome;
    p->ultimo_nome = (char *)ultimo_nome;
    p->dob = (char *)dob;
    p->nacionalidade = internar_nacionalidade(nacionalidade);
    p->destinos_counts = NULL;
    p->owns_strings = 0;
    p->has_details = 1;

    (void)genero;
    (void)email;
    (void)telefone;
    (void)morada;
    (void)foto;

    return p;
}

/**
 * @brief Libera toda a memória associada a um passageiro.
 *
 * @param p Ponteiro para passageiro_t a ser destruído
 */
void passageiro_destruir(passageiro_t *p)
{
    if (!p)
        return;

    if (p->owns_strings)
    {
        free(p->primeiro_nome);
        free(p->ultimo_nome);
        free(p->dob);
    }
    free(p);
}

/**
 * @brief Obtém o primeiro nome do passageiro.
 * @param p Passageiro
 * @return Primeiro nome ou NULL se p for NULL
 */
const char *passageiro_obter_primeiro_nome(const passageiro_t *p)
{
    return p ? p->primeiro_nome : NULL;
}

/**
 * @brief Obtém o último nome do passageiro.
 * @param p Passageiro
 * @return Último nome ou NULL se p for NULL
 */
const char *passageiro_obter_ultimo_nome(const passageiro_t *p)
{
    return p ? p->ultimo_nome : NULL;
}

/**
 * @brief Obtém a data de nascimento do passageiro.
 * @param p Passageiro
 * @return Data de nascimento (string) ou NULL se p for NULL
 */
const char *passageiro_obter_dob(const passageiro_t *p)
{
    return p ? p->dob : NULL;
}

/**
 * @brief Obtém a nacionalidade do passageiro.
 * @param p Passageiro
 * @return Nacionalidade ou NULL se p for NULL
 */
const char *passageiro_obter_nacionalidade(const passageiro_t *p)
{
    return p ? p->nacionalidade : NULL;
}

uint32_t passageiro_obter_document_key(const passageiro_t *p)
{
    return p ? p->doc_key : 0;
}

void passageiro_formatar_documento(const passageiro_t *p, char out[10])
{
    if (!out)
        return;
    if (!p)
    {
        out[0] = '\0';
        return;
    }
    snprintf(out, 10, "%09u", p->doc_key);
}

int passageiro_tem_detalhes(const passageiro_t *p)
{
    return p && p->has_details;
}

void passageiro_definir_detalhes(passageiro_t *p, const char *primeiro_nome,
                                 const char *ultimo_nome, const char *dob)
{
    if (!p || p->has_details)
        return;

    p->primeiro_nome = g_strdup(primeiro_nome ? primeiro_nome : "");
    p->ultimo_nome = g_strdup(ultimo_nome ? ultimo_nome : "");
    p->dob = g_strdup(dob ? dob : "");
    p->owns_strings = 1;
    p->has_details = 1;
}

/**
 * @brief Obtém o gênero do passageiro
 * @param p Passageiro
 * @return Sempre string vazia se p não for NULL, caso contrário NULL
 */
const char *passageiro_obter_genero(const passageiro_t *p)
{
    return p ? "" : NULL;
}

/**
 * @brief Obtém o email do passageiro
 * @param p Passageiro
 * @return Sempre string vazia se p não for NULL, caso contrário NULL
 */
const char *passageiro_obter_email(const passageiro_t *p)
{
    return p ? "" : NULL;
}

/**
 * @brief Obtém o telefone do passageiro
 * @param p Passageiro
 * @return Sempre string vazia se p não for NULL, caso contrário NULL
 */
const char *passageiro_obter_telefone(const passageiro_t *p)
{
    return p ? "" : NULL;
}

/**
 * @brief Obtém a morada do passageiro
 * @param p Passageiro
 * @return Sempre string vazia se p não for NULL, caso contrário NULL
 */
const char *passageiro_obter_morada(const passageiro_t *p)
{
    return p ? "" : NULL;
}

/**
 * @brief Obtém a foto do passageiro
 * @param p Passageiro
 * @return Sempre string vazia se p não for NULL, caso contrário NULL
 */
const char *passageiro_obter_foto(const passageiro_t *p)
{
    return p ? "" : NULL;
}

uint32_t *passageiro_obter_destinos_counts(const passageiro_t *p)
{
    return p ? p->destinos_counts : NULL;
}

void passageiro_definir_destinos_counts(passageiro_t *p, uint32_t *counts)
{
    if (p)
        p->destinos_counts = counts;
}
