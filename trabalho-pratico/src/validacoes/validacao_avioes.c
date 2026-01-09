#include "validacao_avioes.h"
#include "validacao_comum.h"
#include "../../include/utils.h"
#include <string.h>

/** Índice da coluna do identificador do avião */
#define IDX_ID  0
/** Índice da coluna do fabricante */
#define IDX_FAB 1
/** Índice da coluna do modelo */
#define IDX_MOD 2
/** Índice da coluna do ano de fabrico */
#define IDX_ANO 3
/** Índice da coluna da capacidade */
#define IDX_CAP 4
/** Índice da coluna do alcance */
#define IDX_ALC 5

/**
 * @brief Valida uma linha do ficheiro aircrafts.csv e cria um avião.
 *
 * A função executa as seguintes validações:
 * - Verifica a existência do array de colunas
 * - Remove aspas e espaços em branco
 * - Valida campos obrigatórios (ID e fabricante)
 * - Valida capacidade e alcance como inteiros positivos
 * - Valida o ano de fabrico, quando presente
 *
 * Caso todas as validações sejam bem-sucedidas, é criada uma entidade Aviao.
 * Caso contrário, a linha é descartada.
 *
 * @param colunas Array de strings correspondentes às colunas do CSV
 * @return Ponteiro para o Aviao criado ou NULL se a validação falhar
 */
aviao_t *valida_aviao(char **colunas)
{
    if (!colunas)
        return NULL;

    /* Remover aspas das colunas existentes */
    for (int i = 0; i <= IDX_ALC; i++)
    {
        if (colunas[i])
            utils_remove_aspas_somente(colunas[i]);
    }

    /* Verificar se o ano contém espaços */
    if (colunas[IDX_ANO] && *colunas[IDX_ANO])
    {
        if (contem_espacos(colunas[IDX_ANO]))
            return NULL;
    }

    /* Remover espaços em branco */
    for (int i = 0; i <= IDX_ALC; i++)
    {
        if (colunas[i])
            utils_trim(colunas[i]);
    }

    /* Validar campos obrigatórios */
    if (!colunas[IDX_ID] || !*colunas[IDX_ID])
        return NULL;
    if (!colunas[IDX_FAB] || !*colunas[IDX_FAB])
        return NULL;

    /* Validar capacidade e alcance */
    int cap = 0, alc = 0;
    if (!colunas[IDX_CAP] || !validacao_inteiro_positivo(colunas[IDX_CAP], &cap))
        return NULL;
    if (!colunas[IDX_ALC] || !validacao_inteiro_positivo(colunas[IDX_ALC], &alc))
        return NULL;

    /* Validar ano de fabrico (opcional) */
    int ano = 0;
    if (colunas[IDX_ANO] && *colunas[IDX_ANO])
    {
        if (!validacao_ano(colunas[IDX_ANO], &ano))
            return NULL;
    }

    /* Modelo é opcional */
    const char *modelo =
        (colunas[IDX_MOD] && *colunas[IDX_MOD]) ? colunas[IDX_MOD] : "";

    /* Criar avião válido */
    return aviao_criar(
        colunas[IDX_ID],
        colunas[IDX_FAB],
        modelo,
        ano,
        cap,
        alc
    );
}
