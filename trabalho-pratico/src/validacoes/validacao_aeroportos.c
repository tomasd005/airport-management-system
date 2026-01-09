#include "validacao_aeroportos.h"
#include "validacao_comum.h"
#include "../../include/entidades/aeroportos.h"
#include "../../include/utils.h"
#include <string.h>
#include <ctype.h>

/**
 * @brief Verifica se um código de aeroporto é válido.
 *
 * Um código é considerado válido se:
 * - Não for NULL
 * - Tiver exatamente 3 caracteres
 * - Não contiver espaços
 * - Todos os caracteres forem letras maiúsculas (A–Z)
 *
 * @param codigo String com o código do aeroporto
 * @return TRUE se o código for válido, FALSE caso contrário
 */
static inline gboolean codigo_valido(const char *codigo)
{
    if (!codigo || strlen(codigo) != 3 || contem_espacos(codigo))
        return FALSE;

    return g_ascii_isupper(codigo[0]) &&
           g_ascii_isupper(codigo[1]) &&
           g_ascii_isupper(codigo[2]);
}

/**
 * @brief Verifica se o tipo de aeroporto é válido.
 *
 * Apenas são aceites os seguintes tipos:
 * - small_airport
 * - medium_airport
 * - large_airport
 * - heliport
 * - seaplane_base
 *
 * @param tipo String com o tipo do aeroporto
 * @return TRUE se o tipo for válido, FALSE caso contrário
 */
static gboolean tipo_valido(const char *tipo)
{
    if (!tipo)
        return FALSE;

    static const char *tipos[] = {
        "small_airport",
        "medium_airport",
        "large_airport",
        "heliport",
        "seaplane_base"
    };

    for (int i = 0; i < 5; i++)
        if (strcmp(tipo, tipos[i]) == 0)
            return TRUE;

    return FALSE;
}

/**
 * @brief Valida uma linha do ficheiro airports.csv e cria um aeroporto.
 *
 * A função realiza várias validações:
 * - Verifica a existência das 8 colunas obrigatórias
 * - Remove aspas e espaços desnecessários
 * - Valida o código IATA do aeroporto
 * - Valida coordenadas geográficas
 * - Valida o tipo de aeroporto
 * - Garante que campos obrigatórios não estão vazios
 *
 * Se todas as validações forem bem-sucedidas, é criada uma nova entidade
 * Aeroporto. Caso contrário, a linha é descartada.
 *
 * @param colunas Array de strings correspondentes às colunas do CSV
 * @return Ponteiro para o Aeroporto criado ou NULL se a validação falhar
 */
gpointer valida_aeroporto(char **colunas)
{
    if (!colunas)
        return NULL;

    /* Verificar existência de todas as colunas obrigatórias */
    for (int i = 0; i < 8; i++)
        if (!colunas[i])
            return NULL;

    /* Remover aspas */
    for (int i = 0; i < 8; i++)
        utils_remove_aspas_somente(colunas[i]);

    /* Validar código do aeroporto */
    if (contem_espacos(colunas[0]) || !codigo_valido(colunas[0]))
        return NULL;

    /* Remover espaços em branco */
    for (int i = 0; i < 8; i++)
        utils_trim(colunas[i]);

    /* Validar coordenadas e tipo */
    double lat, lon;
    if (!coordenadas_validas(colunas[4], colunas[5], &lat, &lon) ||
        !tipo_valido(colunas[7]))
        return NULL;

    /* Verificar campos obrigatórios */
    if (!colunas[1] || !*colunas[1] ||
        !colunas[2] || !*colunas[2] ||
        !colunas[3] || !*colunas[3])
        return NULL;

    /* Criar aeroporto válido */
    return aeroporto_criar(
        colunas[0],  // código
        colunas[1],  // nome
        colunas[2],  // cidade
        colunas[3],  // país
        lat,
        lon,
        colunas[6],  // continente
        colunas[7]   // tipo
    );
}
