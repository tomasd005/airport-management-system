#ifndef PASSAGEIROS_H
#define PASSAGEIROS_H

#include <stdbool.h>

typedef struct passageiro passageiro_t;

passageiro_t *passageiro_criar(const char *document_number, const char *primeiro_nome, const char *ultimo_nome, const char *dob, const char *nacionalidade, const char *genero, const char *email, const char *telefone, const char *morada, const char *foto);

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
