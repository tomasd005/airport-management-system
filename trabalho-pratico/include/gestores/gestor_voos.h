/**
 * @file gestor_voos.h
 * @brief Interface do gestor de voos com encapsulamento adequado
 */

#ifndef GESTOR_VOOS_H
#define GESTOR_VOOS_H

#include "../entidades/voos.h"

/**
 * @brief Estrutura opaca do gestor de voos
 */
typedef struct gestor_voos gestor_voos_t;

/**
 * @brief Cria um novo gestor de voos
 * @return Ponteiro para o gestor criado
 */
gestor_voos_t *gestor_voos_criar(void);

/**
 * @brief Destrói o gestor e liberta memória
 * @param gestor Gestor a destruir
 */
void gestor_voos_destruir(gestor_voos_t *gestor);

/**
 * @brief Adiciona um voo ao gestor
 * @param gestor Gestor de voos
 * @param voo Voo a adicionar
 */
void gestor_voos_adicionar(gestor_voos_t *gestor, voo_t *voo);

/**
 * @brief Obtém voo por identificador (O(1))
 * @param gestor Gestor de voos
 * @param flight_id Identificador do voo
 * @return Ponteiro para o voo ou NULL se não encontrado
 */
voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id);

/**
 * @brief Conta o número total de voos
 * @param gestor Gestor de voos
 * @return Número de voos no gestor
 */
unsigned int gestor_voos_contar(const gestor_voos_t *gestor);

/**
 * @brief Itera sobre todos os voos
 * @param gestor Gestor de voos
 * @param func Função callback a aplicar (recebe: id, voo, user_data)
 * @param user_data Dados a passar para o callback
 */
void gestor_voos_para_cada(
    gestor_voos_t *gestor,
    void (*func)(const char *id, voo_t *voo, void *user_data),
    void *user_data);

/**
 * @brief Itera sobre voos atrasados
 * @param gestor Gestor de voos
 * @param func Função callback a aplicar (recebe: voo, user_data)
 * @param user_data Dados a passar para o callback
 */
void gestor_voos_para_cada_atrasado(
    gestor_voos_t *gestor,
    void (*func)(voo_t *voo, void *user_data),
    void *user_data);

/**
 * @brief Itera sobre voos que partem de um aeroporto
 * @param gestor Gestor de voos
 * @param origin Código IATA do aeroporto de origem
 * @param func Função callback a aplicar (recebe: voo, user_data)
 * @param user_data Dados a passar para o callback
 */
void gestor_voos_para_cada_origem(
    gestor_voos_t *gestor,
    const char *origin,
    void (*func)(voo_t *voo, void *user_data),
    void *user_data);

/**
 * @brief Itera sobre voos que chegam a um aeroporto
 * @param gestor Gestor de voos
 * @param destination Código IATA do aeroporto de destino
 * @param func Função callback a aplicar (recebe: voo, user_data)
 * @param user_data Dados a passar para o callback
 */
void gestor_voos_para_cada_destino(
    gestor_voos_t *gestor,
    const char *destination,
    void (*func)(voo_t *voo, void *user_data),
    void *user_data);

/**
 * @brief Obtém data de partida de um voo
 * @param gestor Gestor de voos
 * @param flight_id Identificador do voo
 * @return String com a data de partida ou NULL se voo não existe
 */
const char *gestor_voos_obter_departure(
    gestor_voos_t *gestor,
    const char *flight_id);

/**
 * @brief Carrega voos de ficheiro CSV
 * @param gestor Gestor de voos
 * @param ficheiro_csv Caminho do ficheiro CSV
 */
void gestor_voos_carregar(gestor_voos_t *gestor, const char *ficheiro_csv);

#endif /* GESTOR_VOOS_H */