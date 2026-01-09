#include "aeroportos.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>

/**
 * @struct aeroporto
 * @brief Representa um aeroporto com informações básicas e contadores.
 */
struct aeroporto
{
    char *codigo;  
    char *nome;     
    char *cidade;   
    char *pais;     
    char *tipo;     
    int partidas;  
    int chegadas;   
};

/**
 * @brief Cria um novo aeroporto com os dados fornecidos.
 *
 * Inicializa os contadores de partidas e chegadas com zero.
 *
 * @param codigo Código do aeroporto 
 * @param nome Nome do aeroporto 
 * @param cidade Cidade do aeroporto 
 * @param pais País do aeroporto 
 * @param latitude Latitude do aeroporto
 * @param longitude Longitude do aeroporto 
 * @param icao Código ICAO do aeroporto 
 * @param tipo Tipo do aeroporto
 * @return Ponteiro para aeroporto_t recém-criado, ou NULL em caso de erro
 */
aeroporto_t *aeroporto_criar(const char *codigo, const char *nome, const char *cidade,
                             const char *pais, double latitude, double longitude,
                             const char *icao, const char *tipo)
{
    if (!codigo || !nome || !cidade || !pais || !tipo)
        return NULL;

    aeroporto_t *a = malloc(sizeof(aeroporto_t));
    if (!a)
        return NULL;

    a->codigo = g_strdup(codigo);
    a->nome = g_strdup(nome);
    a->cidade = g_strdup(cidade);
    a->pais = g_strdup(pais);
    a->tipo = g_strdup(tipo);
    a->partidas = 0;
    a->chegadas = 0;

    if (!a->codigo || !a->nome || !a->cidade || !a->pais || !a->tipo)
    {
        aeroporto_destruir(a);
        return NULL;
    }

    return a;
}

/**
 * @brief Libera toda a memória associada a um aeroporto.
 *
 * @param a Ponteiro para aeroporto_t a ser destruído
 */
void aeroporto_destruir(aeroporto_t *a)
{
    if (!a)
        return;

    free(a->codigo);
    free(a->nome);
    free(a->cidade);
    free(a->pais);
    free(a->tipo);
    free(a);
}

/**
 * @brief Obtém o código do aeroporto.
 * @param a Aeroporto
 * @return Código (string) ou NULL se a for NULL
 */
const char *aeroporto_obter_codigo(const aeroporto_t *a) { return a ? a->codigo : NULL; }

/**
 * @brief Obtém o nome do aeroporto.
 * @param a Aeroporto
 * @return Nome (string) ou NULL se a for NULL
 */
const char *aeroporto_obter_nome(const aeroporto_t *a) { return a ? a->nome : NULL; }

/**
 * @brief Obtém a cidade do aeroporto.
 * @param a Aeroporto
 * @return Cidade (string) ou NULL se a for NULL
 */
const char *aeroporto_obter_cidade(const aeroporto_t *a) { return a ? a->cidade : NULL; }

/**
 * @brief Obtém o país do aeroporto.
 * @param a Aeroporto
 * @return País (string) ou NULL se a for NULL
 */
const char *aeroporto_obter_pais(const aeroporto_t *a) { return a ? a->pais : NULL; }

/**
 * @brief Obtém a latitude do aeroporto.
 * @param a Aeroporto
 * @return Sempre 0.0
 */
double aeroporto_obter_latitude(const aeroporto_t *a) { return 0.0; }

/**
 * @brief Obtém a longitude do aeroporto.
 * @param a Aeroporto
 * @return Sempre 0.0
 */
double aeroporto_obter_longitude(const aeroporto_t *a) { return 0.0; }

/**
 * @brief Obtém o código ICAO do aeroporto.
 * @param a Aeroporto
 * @return Sempre string vazia
 */
const char *aeroporto_obter_icao(const aeroporto_t *a) { return a ? "" : NULL; }

/**
 * @brief Obtém o tipo do aeroporto.
 * @param a Aeroporto
 * @return Tipo (string) ou NULL se a for NULL
 */
const char *aeroporto_obter_tipo(const aeroporto_t *a) { return a ? a->tipo : NULL; }

/**
 * @brief Obtém o número de partidas registradas.
 * @param a Aeroporto
 * @return Número de partidas, ou 0 se a for NULL
 */
int aeroporto_obter_partidas(const aeroporto_t *a) { return a ? a->partidas : 0; }

/**
 * @brief Obtém o número de chegadas registradas.
 * @param a Aeroporto
 * @return Número de chegadas, ou 0 se a for NULL
 */
int aeroporto_obter_chegadas(const aeroporto_t *a) { return a ? a->chegadas : 0; }

/**
 * @brief Incrementa o contador de partidas do aeroporto.
 * @param a Aeroporto
 * @param delta Valor a adicionar ao contador de partidas
 */
void aeroporto_incrementar_partidas(aeroporto_t *a, int delta)
{
    if (!a)
        return;
    a->partidas += delta;
}

/**
 * @brief Incrementa o contador de chegadas do aeroporto.
 * @param a Aeroporto
 * @param delta Valor a adicionar ao contador de chegadas
 */
void aeroporto_incrementar_chegadas(aeroporto_t *a, int delta)
{
    if (!a)
        return;
    a->chegadas += delta;
}
