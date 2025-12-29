/**
 * @file gestor_passageiros.h
 * @brief Interface do gestor de passageiros
 */

#ifndef GESTOR_PASSAGEIROS_H
#define GESTOR_PASSAGEIROS_H

#include "../entidades/passageiros.h"
#include <glib.h>

/**
 * @brief Estrutura opaca do gestor de passageiros
 */
typedef struct gestor_passageiros gestor_passageiros_t;

/**
 * @brief Cria um novo gestor de passageiros
 * @return Ponteiro para o gestor criado
 */
gestor_passageiros_t *gestor_passageiros_criar(void);

/**
 * @brief Destrói o gestor de passageiros e liberta memória
 * @param gestor Gestor a destruir
 */
void gestor_passageiros_destruir(gestor_passageiros_t *gestor);

/**
 * @brief Adiciona um passageiro ao gestor
 * @param gestor Gestor de passageiros
 * @param p Passageiro a adicionar
 */
void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *p);

/**
 * @brief Obtém passageiro por número de documento (O(1))
 * @param gestor Gestor de passageiros
 * @param document_number Número do documento
 * @return Ponteiro para o passageiro ou NULL se não encontrado
 */
passageiro_t *gestor_passageiros_obter_por_documento(
    gestor_passageiros_t *gestor,
    const char *document_number);

/**
 * @brief Obtém lista de passageiros por nacionalidade (O(1))
 * @param gestor Gestor de passageiros
 * @param nacionalidade Nacionalidade a procurar
 * @return GPtrArray com passageiros ou NULL se não houver
 */
GPtrArray *gestor_passageiros_obter_por_nacionalidade(
    gestor_passageiros_t *gestor,
    const char *nacionalidade);

/**
 * @brief Retorna o número total de passageiros
 * @param gestor Gestor de passageiros
 * @return Número de passageiros
 */
unsigned int gestor_passageiros_numero(gestor_passageiros_t *gestor);

/**
 * @brief Carrega passageiros de ficheiro CSV
 * @param gestor Gestor de passageiros
 * @param ficheiro_csv Caminho do ficheiro CSV
 */
void gestor_passageiros_carregar(gestor_passageiros_t *gestor, const char *ficheiro_csv);

#endif /* GESTOR_PASSAGEIROS_H */