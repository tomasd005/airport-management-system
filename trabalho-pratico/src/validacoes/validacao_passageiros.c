#include "validacao_passageiros.h"
#include "validacao_comum.h"
#include "../../include/utils.h"
#include <string.h>
#include <ctype.h>

/** Índices das colunas do CSV de passageiros */
#define IDX_DOC 0
#define IDX_FIRST 1
#define IDX_LAST 2
#define IDX_DOB 3
#define IDX_NAT 4
#define IDX_GEN 5
#define IDX_EMAIL 6
#define IDX_PHONE 7
#define IDX_ADDR 8
#define IDX_PHOTO 9

/**
 * @brief Valida o número de documento do passageiro.
 *
 * Formato esperado: 9 dígitos sem espaços.
 *
 * @param doc Número de documento
 * @return TRUE se válido, FALSE caso contrário
 */
static inline gboolean valida_document_number(const char *doc)
{
    if (!doc || strlen(doc) != 9 || contem_espacos(doc))
        return FALSE;

    for (int i = 0; i < 9; i++)
        if (!isdigit(doc[i]))
            return FALSE;
    return TRUE;
}

/**
 * @brief Valida o género do passageiro.
 *
 * Valores válidos: 'M', 'F', 'O'.
 *
 * @param gen String do género
 * @return TRUE se válido, FALSE caso contrário
 */
static inline gboolean valida_genero(const char *gen)
{
    return gen && strlen(gen) == 1 && (gen[0] == 'M' || gen[0] == 'F' || gen[0] == 'O');
}

/**
 * @brief Valida o email do passageiro.
 *
 * Regras básicas:
 * - Contém exatamente um '@'
 * - Username antes do '@' contém letras minúsculas, números ou '.'
 * - Domínio contém letras minúsculas
 * - Extensão tem 2 a 3 letras minúsculas
 *
 * @param email String do email
 * @return TRUE se válido, FALSE caso contrário
 */
static gboolean valida_email(const char *email)
{
    if (!email)
        return FALSE;

    const char *at = strchr(email, '@');
    if (!at || at == email)
        return FALSE;

    for (const char *p = email; p < at; p++)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '.'))
            return FALSE;

    const char *domain = at + 1;
    const char *dot = strchr(domain, '.');
    if (!dot || dot == domain)
        return FALSE;

    for (const char *p = domain; p < dot; p++)
        if (!(*p >= 'a' && *p <= 'z'))
            return FALSE;

    const char *ext = dot + 1;
    int len = strlen(ext);
    if (len < 2 || len > 3 || strchr(ext, '.'))
        return FALSE;

    for (int i = 0; i < len; i++)
        if (!(ext[i] >= 'a' && ext[i] <= 'z'))
            return FALSE;

    return TRUE;
}

/**
 * @brief Valida todas as colunas de um passageiro.
 *
 * Realiza validação do número de documento, data de nascimento,
 * género, email e campos obrigatórios.
 *
 * @param colunas Array de strings contendo os campos do passageiro
 * @return ponteiro para um passageiro criado se válido, NULL caso contrário
 */
passageiro_t *valida_passageiro(char **colunas)
{
    if (!colunas)
        return NULL;

    /* Remove aspas das colunas */
    for (int i = 0; i <= IDX_PHOTO; i++)
        if (colunas[i])
            utils_remove_aspas(colunas[i]);

    if (!valida_document_number(colunas[IDX_DOC]))
        return NULL;

    /* Remove espaços em excesso */
    for (int i = 0; i <= IDX_PHOTO; i++)
        if (colunas[i])
            utils_trim(colunas[i]);

    /* Valida data, género e email */
    if (!validacao_data_passado(colunas[IDX_DOB]) ||
        !valida_genero(colunas[IDX_GEN]) ||
        !valida_email(colunas[IDX_EMAIL]))
        return NULL;

    /* Campos obrigatórios */
    if (!colunas[IDX_FIRST] || !*colunas[IDX_FIRST] ||
        !colunas[IDX_LAST] || !*colunas[IDX_LAST] ||
        !colunas[IDX_NAT] || !*colunas[IDX_NAT] ||
        !colunas[IDX_PHONE] || !*colunas[IDX_PHONE] ||
        !colunas[IDX_ADDR] || !*colunas[IDX_ADDR])
        return NULL;

    return passageiro_criar(colunas[IDX_DOC], colunas[IDX_FIRST], colunas[IDX_LAST],
                            colunas[IDX_DOB], colunas[IDX_NAT], colunas[IDX_GEN],
                            colunas[IDX_EMAIL], colunas[IDX_PHONE], colunas[IDX_ADDR],
                            colunas[IDX_PHOTO]);
}
