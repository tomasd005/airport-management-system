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
gboolean aviao_validar_sintatica(char **colunas, int *out_ano, int *out_cap, int *out_alc,
                                 const char **out_modelo)
{
    if (!colunas || !out_ano || !out_cap || !out_alc || !out_modelo)
        return FALSE;

    for (int i = 0; i <= IDX_ALC; i++)
        if (colunas[i])
            utils_remove_aspas_somente(colunas[i]);

    if (colunas[IDX_ANO] && *colunas[IDX_ANO]) {
        if (contem_espacos(colunas[IDX_ANO]))
            return FALSE;
    }

    for (int i = 0; i <= IDX_ALC; i++)
        if (colunas[i])
            utils_trim(colunas[i]);

    if (!colunas[IDX_ID] || !*colunas[IDX_ID])
        return FALSE;
    if (!colunas[IDX_FAB] || !*colunas[IDX_FAB])
        return FALSE;

    if (!colunas[IDX_CAP] || !validacao_inteiro_positivo(colunas[IDX_CAP], out_cap))
        return FALSE;
    if (!colunas[IDX_ALC] || !validacao_inteiro_positivo(colunas[IDX_ALC], out_alc))
        return FALSE;

    *out_ano = 0;
    if (colunas[IDX_ANO] && *colunas[IDX_ANO]) {
        if (!validacao_ano(colunas[IDX_ANO], out_ano))
            return FALSE;
    }

    *out_modelo = (colunas[IDX_MOD] && *colunas[IDX_MOD]) ? colunas[IDX_MOD] : "";
    return TRUE;
}

gboolean aviao_validar_logica(char **colunas)
{
    (void)colunas;
    return TRUE;
}

aviao_t *valida_aviao(char **colunas)
{
    int ano = 0, cap = 0, alc = 0;
    const char *modelo = "";

    if (!aviao_validar_sintatica(colunas, &ano, &cap, &alc, &modelo))
        return NULL;
    if (!aviao_validar_logica(colunas))
        return NULL;

    return aviao_criar(colunas[IDX_ID], colunas[IDX_FAB], modelo, ano, cap, alc);
}
