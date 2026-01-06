#ifndef GESTOR_PASSAGEIROS_H
#define GESTOR_PASSAGEIROS_H

#include "../entidades/passageiros.h"
#include <glib.h>

typedef struct gestor_passageiros gestor_passageiros_t;

gestor_passageiros_t *gestor_passageiros_criar(void);
void gestor_passageiros_destruir(gestor_passageiros_t *gestor);
void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *p);
passageiro_t *gestor_passageiros_obter_por_documento(gestor_passageiros_t *gestor, const char *document_number);
GPtrArray *gestor_passageiros_obter_por_nacionalidade(gestor_passageiros_t *gestor, const char *nacionalidade);
unsigned int gestor_passageiros_numero(gestor_passageiros_t *gestor);
void gestor_passageiros_carregar(gestor_passageiros_t *gestor, const char *ficheiro_csv);
void gestor_passageiros_para_cada(gestor_passageiros_t *gestor, void (*callback)(passageiro_t *, void *), void *user_data);

#endif
