#ifndef GESTOR_QUERIES_H
#define GESTOR_QUERIES_H

/**
 * @file gestor_queries.h
 * @brief Interface do gestor de queries do programa.
 *
 * Permite criar, destruir e processar queries usando os gestores
 * de aeroportos, aviões, voos, passageiros e reservas.
 */

typedef struct gestor_queries gestor_queries_t;
typedef struct gestor_aeroportos gestor_aeroportos_t;
typedef struct gestor_avioes gestor_avioes_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_passageiros gestor_passageiros_t;
typedef struct gestor_reservas gestor_reservas_t;

/**
 * @brief Cria um novo gestor de queries.
 *
 * @param aeroportos Ponteiro para o gestor de aeroportos.
 * @param avioes Ponteiro para o gestor de aviões.
 * @param voos Ponteiro para o gestor de voos.
 * @param passageiros Ponteiro para o gestor de passageiros.
 * @param reservas Ponteiro para o gestor de reservas.
 * @return Ponteiro para o gestor de queries criado, ou NULL em caso de falha.
 */
gestor_queries_t *gestor_queries_criar(
    gestor_aeroportos_t *aeroportos,
    gestor_avioes_t *avioes,
    gestor_voos_t *voos,
    gestor_passageiros_t *passageiros,
    gestor_reservas_t *reservas);

/**
 * @brief Destrói o gestor de queries.
 * @param gestor Gestor a destruir (aceita NULL).
 */
void gestor_queries_destruir(gestor_queries_t *gestor);

/**
 * @brief Processa um ficheiro de comandos (queries).
 * @param gestor Gestor de queries.
 * @param ficheiro_input Caminho do ficheiro de comandos.
 */
void gestor_queries_processar_ficheiro(
    gestor_queries_t *gestor,
    const char *ficheiro_input);

#endif
