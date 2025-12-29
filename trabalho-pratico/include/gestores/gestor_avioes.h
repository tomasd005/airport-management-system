/**
 * @file gestor_avioes.h
 * @brief Interface do gestor de aviões com encapsulamento adequado
 */

#ifndef GESTOR_AVIOES_H
#define GESTOR_AVIOES_H

#include "../entidades/avioes.h"

/**
 * @brief Estrutura opaca do gestor de aviões
 */
typedef struct gestor_avioes gestor_avioes_t;

/**
 * @brief Cria um novo gestor de aviões
 * @return Ponteiro para o gestor criado
 */
gestor_avioes_t *gestor_avioes_criar(void);

/**
 * @brief Destrói o gestor e liberta memória
 * @param gestor Gestor a destruir
 */
void gestor_avioes_destruir(gestor_avioes_t *gestor);

/**
 * @brief Adiciona um avião ao gestor
 * @param gestor Gestor de aviões
 * @param aviao Avião a adicionar
 */
void gestor_avioes_adicionar(gestor_avioes_t *gestor, aviao_t *aviao);

/**
 * @brief Obtém avião por identificador (O(1))
 * @param gestor Gestor de aviões
 * @param identificador Identificador do avião (tail number)
 * @return Ponteiro para o avião ou NULL se não encontrado
 */
aviao_t *gestor_avioes_obter_por_id(gestor_avioes_t *gestor, const char *identificador);

/**
 * @brief Conta o número total de aviões
 * @param gestor Gestor de aviões
 * @return Número de aviões no gestor
 */
unsigned int gestor_avioes_contar(const gestor_avioes_t *gestor);

/**
 * @brief Itera sobre todos os aviões
 * @param gestor Gestor de aviões
 * @param func Função callback a aplicar (recebe: id, aviao, user_data)
 * @param user_data Dados a passar para o callback
 *
 * Exemplo de uso:
 * @code
 * void processar(const char *id, aviao_t *aviao, void *ctx) {
 *     printf("Avião: %s\n", id);
 * }
 * gestor_avioes_para_cada(gestor, processar, NULL);
 * @endcode
 */
void gestor_avioes_para_cada(
    gestor_avioes_t *gestor,
    void (*func)(const char *id, aviao_t *aviao, void *user_data),
    void *user_data);

/**
 * @brief Carrega aviões de ficheiro CSV
 * @param gestor Gestor de aviões
 * @param ficheiro_csv Caminho do ficheiro CSV
 */
void gestor_avioes_carregar(gestor_avioes_t *gestor, const char *ficheiro_csv);

#endif /* GESTOR_AVIOES_H */