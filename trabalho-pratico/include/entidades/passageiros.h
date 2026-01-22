#ifndef PASSAGEIROS_H
#define PASSAGEIROS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file passageiros.h
 * @brief Interface para manipulação de passageiros.
 *
 * Define a estrutura e funções para criar, destruir e obter informações
 * sobre passageiros, incluindo dados pessoais, contato e fotografia.
 */

/**
 * @brief Estrutura opaca que representa um passageiro.
 */
typedef struct passageiro passageiro_t;

/**
 * @brief Cria um passageiro.
 *
 * @param document_number Número do documento do passageiro (único, 9 dígitos).
 * @param primeiro_nome Primeiro nome do passageiro.
 * @param ultimo_nome Último nome do passageiro.
 * @param dob Data de nascimento no formato YYYY-MM-DD.
 * @param nacionalidade Nacionalidade do passageiro.
 * @param genero Gênero do passageiro ('M', 'F' ou 'O').
 * @param email Email do passageiro.
 * @param telefone Número de telefone do passageiro.
 * @param morada Morada completa do passageiro.
 * @param foto Caminho ou identificador da foto do passageiro.
 * @return Ponteiro para o passageiro criado ou NULL em caso de erro.
 */
passageiro_t *passageiro_criar(const char *document_number, const char *primeiro_nome, const char *ultimo_nome, const char *dob, const char *nacionalidade, const char *genero, const char *email, const char *telefone, const char *morada, const char *foto);

/**
 * @brief Destrói um passageiro e libera a memória associada.
 *
 * @param p Ponteiro para o passageiro a destruir.
 */
void passageiro_destruir(passageiro_t *p);

/**
 * @brief Obtém o número do documento do passageiro.
 *
 * @param p Passageiro.
 * @return Número de documento ou NULL se p for NULL.
 */
const char *passageiro_obter_document_number(const passageiro_t *p);

/**
 * @brief Obtém o primeiro nome do passageiro.
 *
 * @param p Passageiro.
 * @return Primeiro nome ou NULL se p for NULL.
 */
const char *passageiro_obter_primeiro_nome(const passageiro_t *p);

/**
 * @brief Obtém o último nome do passageiro.
 *
 * @param p Passageiro.
 * @return Último nome ou NULL se p for NULL.
 */
const char *passageiro_obter_ultimo_nome(const passageiro_t *p);

/**
 * @brief Obtém a data de nascimento do passageiro.
 *
 * @param p Passageiro.
 * @return Data de nascimento (YYYY-MM-DD) ou NULL se p for NULL.
 */
const char *passageiro_obter_dob(const passageiro_t *p);

/**
 * @brief Obtém a nacionalidade do passageiro.
 *
 * @param p Passageiro.
 * @return Nacionalidade ou NULL se p for NULL.
 */
const char *passageiro_obter_nacionalidade(const passageiro_t *p);

/**
 * @brief Obtém o género do passageiro.
 *
 * @param p Passageiro.
 * @return Género ou NULL se p for NULL.
 */
const char *passageiro_obter_genero(const passageiro_t *p);

/**
 * @brief Obtém o email do passageiro.
 *
 * @param p Passageiro.
 * @return Email ou NULL se p for NULL.
 */
const char *passageiro_obter_email(const passageiro_t *p);

/**
 * @brief Obtém o telefone do passageiro.
 *
 * @param p Passageiro.
 * @return Telefone ou NULL se p for NULL.
 */
const char *passageiro_obter_telefone(const passageiro_t *p);

/**
 * @brief Obtém a morada do passageiro.
 *
 * @param p Passageiro.
 * @return Morada ou NULL se p for NULL.
 */
const char *passageiro_obter_morada(const passageiro_t *p);

/**
 * @brief Obtém o identificador da foto do passageiro.
 *
 * @param p Passageiro.
 * @return Foto ou NULL se p for NULL.
 */
const char *passageiro_obter_foto(const passageiro_t *p);

/**
 * @brief Obtém o mapa de destinos por nacionalidade associado ao passageiro.
 *
 * @param p Passageiro.
 * @return Ponteiro para o array de contagens ou NULL se não definido.
 */
uint32_t *passageiro_obter_destinos_counts(const passageiro_t *p);

/**
 * @brief Define o mapa de destinos por nacionalidade no passageiro.
 *
 * @param p Passageiro.
 * @param counts Ponteiro para o array de contagens partilhado pela nacionalidade.
 */
void passageiro_definir_destinos_counts(passageiro_t *p, uint32_t *counts);

#endif /* PASSAGEIROS_H */
