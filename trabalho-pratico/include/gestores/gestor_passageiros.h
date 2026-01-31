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
 * @return Gestor alocado, ou NULL em erro de memória.
 */
gestor_passageiros_t *gestor_passageiros_criar(void);

/**
 * @brief Destrói o gestor e liberta todos os passageiros.
 * @param gestor Gestor a destruir (aceita NULL).
 */
void gestor_passageiros_destruir(gestor_passageiros_t *gestor);

/**
 * @brief Adiciona um passageiro ao gestor.
 * @param gestor Gestor de passageiros.
 * @param p Passageiro a inserir (ownership transferido).
 */
void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *p);

/**
 * @brief Obtém passageiro por número de documento.
 * @param gestor Gestor de passageiros.
 * @param document_number Número de documento.
 * @return Ponteiro para o passageiro, ou NULL se não existir.
 */
passageiro_t *gestor_passageiros_obter_por_documento(gestor_passageiros_t *gestor, const char *document_number);

/**
 * @brief Obtém passageiro por chave de documento compacta.
 * @param gestor Gestor de passageiros.
 * @param key Chave compacta do documento.
 * @return Ponteiro para o passageiro, ou NULL se não existir.
 */
passageiro_t *gestor_passageiros_obter_por_documento_key(gestor_passageiros_t *gestor, uint32_t key);

/**
 * @brief Devolve o número de passageiros.
 * @param gestor Gestor de passageiros.
 * @return Total de passageiros.
 */
unsigned int gestor_passageiros_numero(gestor_passageiros_t *gestor);

/**
 * @brief Carrega passageiros a partir de CSV.
 * @param gestor Gestor de passageiros.
 * @param ficheiro_csv Caminho para o CSV.
 */
void gestor_passageiros_carregar(gestor_passageiros_t *gestor, const char *ficheiro_csv);

/**
 * @brief Carrega apenas detalhes completos para passageiros selecionados.
 *
 * @param gestor Gestor de passageiros.
 * @param doc_keys Set com chaves de documentos a completar.
 */
void gestor_passageiros_carregar_detalhes(gestor_passageiros_t *gestor, GHashTable *doc_keys);

#endif
