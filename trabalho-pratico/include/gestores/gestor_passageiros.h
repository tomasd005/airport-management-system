#ifndef GESTOR_PASSAGEIROS_H
#define GESTOR_PASSAGEIROS_H

#include <glib.h>
#include "../entidades/passageiros.h"

typedef struct gestor_passageiros gestor_passageiros_t;

gestor_passageiros_t *gestor_passageiros_criar(void);
void gestor_passageiros_destruir(gestor_passageiros_t *gestor);
void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *passageiro);
passageiro_t *gestor_passageiros_obter_por_documento(gestor_passageiros_t *gestor, const char *document_number);
unsigned int gestor_passageiros_numero(gestor_passageiros_t *gestor);

// Função de carregamento (antes em _parser.h)
void gestor_passageiros_carregar(gestor_passageiros_t *gestor, const char *ficheiro_csv);

#endif