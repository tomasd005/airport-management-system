#include "avioes.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>

/**
 * @struct aviao
 * @brief Representa um avião com informações básicas e contagem de voos.
 */
struct aviao
{
    char *identificador; 
    char *fabricante;   
    char *modelo;        
    int contagem_voos;   
};

/**
 * @brief Cria um novo avião com os dados fornecidos.
 *
 * Inicializa a contagem de voos com zero.
 *
 * @param identificador Identificador único do avião
 * @param fabricante Fabricante do avião 
 * @param modelo Modelo do avião 
 * @param ano Ano de fabricação 
 * @param capacidade Capacidade do avião 
 * @param alcance_km Alcance em km 
 * @return Ponteiro para aviao_t recém-criado, ou NULL em caso de erro
 */
aviao_t *aviao_criar(const char *identificador, const char *fabricante,
                     const char *modelo, int ano, int capacidade, int alcance_km)
{
    if (!identificador || !fabricante || !modelo)
        return NULL;

    aviao_t *a = malloc(sizeof(aviao_t));
    if (!a)
        return NULL;

    a->identificador = g_strdup(identificador);
    a->fabricante = g_strdup(fabricante);
    a->modelo = g_strdup(modelo);
    a->contagem_voos = 0;

    if (!a->identificador || !a->fabricante || !a->modelo)
    {
        aviao_destruir(a);
        return NULL;
    }

    return a;
}

/**
 * @brief Libera toda a memória associada a um avião.
 *
 * @param a Ponteiro para aviao_t a ser destruído
 */
void aviao_destruir(aviao_t *a)
{
    if (!a)
        return;

    free(a->identificador);
    free(a->fabricante);
    free(a->modelo);
    free(a);
}

/**
 * @brief Obtém o identificador do avião.
 * @param a Avião
 * @return Identificador (string) ou NULL se a for NULL
 */
const char *aviao_obter_identificador(const aviao_t *a)
{
    return a ? a->identificador : NULL;
}

/**
 * @brief Obtém o fabricante do avião.
 * @param a Avião
 * @return Fabricante (string) ou NULL se a for NULL
 */
const char *aviao_obter_fabricante(const aviao_t *a)
{
    return a ? a->fabricante : NULL;
}

/**
 * @brief Obtém o modelo do avião.
 * @param a Avião
 * @return Modelo (string) ou NULL se a for NULL
 */
const char *aviao_obter_modelo(const aviao_t *a)
{
    return a ? a->modelo : NULL;
}

/**
 * @brief Obtém o ano do avião.
 * @param a Avião
 * @return Sempre 0
 */
int aviao_obter_ano(const aviao_t *a)
{
    return 0;
}

/**
 * @brief Obtém a capacidade do avião.
 * @param a Avião
 * @return Sempre 0
 */
int aviao_obter_capacidade(const aviao_t *a)
{
    return 0;
}

/**
 * @brief Obtém o alcance em km do avião.
 * @param a Avião
 * @return Sempre 0
 */
int aviao_obter_alcance_km(const aviao_t *a)
{
    return 0;
}

/**
 * @brief Obtém o número de voos realizados pelo avião.
 * @param a Avião
 * @return Contagem de voos ou 0 se a for NULL
 */
int aviao_obter_contagem_voos(const aviao_t *a)
{
    return a ? a->contagem_voos : 0;
}

/**
 * @brief Incrementa a contagem de voos do avião.
 * @param a Avião
 * @param delta Valor a adicionar à contagem de voos
 */
void aviao_incrementar_contagem_voos(aviao_t *a, int delta)
{
    if (!a)
        return;
    a->contagem_voos += delta;
}