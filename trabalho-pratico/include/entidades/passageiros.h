#ifndef PASSAGEIROS_H
#define PASSAGEIROS_H

#include <stdbool.h>

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

const char *passageiro_obter_document_number(const passageiro_t *p);
const char *passageiro_obter_primeiro_nome(const passageiro_t *p);
const char *passageiro_obter_ultimo_nome(const passageiro_t *p);
const char *passageiro_obter_dob(const passageiro_t *p);
const char *passageiro_obter_nacionalidade(const passageiro_t *p);
const char *passageiro_obter_genero(const passageiro_t *p);
const char *passageiro_obter_email(const passageiro_t *p);
const char *passageiro_obter_telefone(const passageiro_t *p);
const char *passageiro_obter_morada(const passageiro_t *p);
const char *passageiro_obter_foto(const passageiro_t *p);

#endif /* PASSAGEIROS_H */
