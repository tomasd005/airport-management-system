#ifndef GESTOR_PASSAGEIROS_H
#define GESTOR_PASSAGEIROS_H

#include "../entidades/passageiros.h"
#include <glib.h>

/**
 * @file gestor_passageiros.h
 * @brief Gestão centralizada de passageiros.
 *
 * Este módulo permite criar, destruir e gerir um conjunto de passageiros,
 * incluindo inserção, consulta, carregamento a partir de CSV e iteração.
 */

typedef struct gestor_passageiros gestor_passageiros_t;

/**
 * @brief Cria um gestor de passageiros.
 *
 * @return Ponteiro para o gestor criado.
 */
gestor_passageiros_t *gestor_passageiros_criar(void);

/**
 * @brief Destroi o gestor de passageiros e liberta recursos.
 *
 * @param gestor Gestor de passageiros.
 */
void gestor_passageiros_destruir(gestor_passageiros_t *gestor);

/**
 * @brief Adiciona um passageiro ao gestor.
 *
 * @param gestor Gestor de passageiros.
 * @param p Passageiro a adicionar.
 */
void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *p);

/**
 * @brief Obtém um passageiro pelo número de documento.
 *
 * @param gestor Gestor de passageiros.
 * @param document_number Documento do passageiro.
 * @return Ponteiro para o passageiro ou NULL se não existir.
 */
passageiro_t *gestor_passageiros_obter_por_documento(gestor_passageiros_t *gestor, const char *document_number);

/**
 * @brief Obtém um passageiro pela chave numérica do documento.
 *
 * @param gestor Gestor de passageiros.
 * @param document_key Documento convertido em chave numérica.
 * @return Ponteiro para o passageiro ou NULL se não existir.
 */
passageiro_t *gestor_passageiros_obter_por_documento_key(gestor_passageiros_t *gestor, unsigned int document_key);

/**
 * @brief Retorna o número de passageiros no gestor.
 *
 * @param gestor Gestor de passageiros.
 * @return Número de passageiros.
 */
unsigned int gestor_passageiros_numero(gestor_passageiros_t *gestor);

/**
 * @brief Carrega passageiros de um ficheiro CSV.
 *
 * @param gestor Gestor de passageiros.
 * @param ficheiro_csv Caminho do CSV.
 */
void gestor_passageiros_carregar(gestor_passageiros_t *gestor, const char *ficheiro_csv);

/**
 * @brief Aplica um callback a cada passageiro.
 *
 * @param gestor Gestor de passageiros.
 * @param callback Função a executar por passageiro.
 * @param user_data Dados do utilizador.
 */
void gestor_passageiros_para_cada(gestor_passageiros_t *gestor, void (*callback)(passageiro_t *, void *), void *user_data);

#endif
