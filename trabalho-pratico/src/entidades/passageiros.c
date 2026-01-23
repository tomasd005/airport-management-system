#include "passageiros.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdint.h>

/**
 * @struct passageiro
 * @brief Representa um passageiro com dados pessoais.
 */
struct passageiro
{
    char *document_number; 
    char *primeiro_nome;   
    char *ultimo_nome;      
    char *dob;              
    char *nacionalidade;    
    uint32_t *destinos_counts;
    unsigned char owns_strings;
};

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

    p->document_number = g_strdup(document_number);
    p->primeiro_nome = g_strdup(primeiro_nome);
    p->ultimo_nome = g_strdup(ultimo_nome);
    p->dob = g_strdup(dob);
    p->nacionalidade = g_strdup(nacionalidade);
    p->destinos_counts = NULL;
    p->owns_strings = 1;

    if (!p->document_number || !p->primeiro_nome || !p->ultimo_nome ||
        !p->dob || !p->nacionalidade)
    {
        passageiro_destruir(p);
        return NULL;
    }

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

    p->document_number = (char *)document_number;
    p->primeiro_nome = (char *)primeiro_nome;
    p->ultimo_nome = (char *)ultimo_nome;
    p->dob = (char *)dob;
    p->nacionalidade = (char *)nacionalidade;
    p->destinos_counts = NULL;
    p->owns_strings = 0;

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
        free(p->document_number);
        free(p->primeiro_nome);
        free(p->ultimo_nome);
        free(p->dob);
        free(p->nacionalidade);
    }
    free(p);
}

/**
 * @brief Obtém o número do documento do passageiro.
 * @param p Passageiro
 * @return Número do documento (string) ou NULL se p for NULL
 */
const char *passageiro_obter_document_number(const passageiro_t *p)
{
    return p ? p->document_number : NULL;
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
