#ifndef GESTOR_AVIOES_H
#define GESTOR_AVIOES_H

#include <glib.h>
#include "../entidades/avioes.h"

typedef struct gestor_avioes gestor_avioes_t;

gestor_avioes_t *gestor_avioes_criar(void);
void gestor_avioes_destruir(gestor_avioes_t *gestor);
void gestor_avioes_adicionar(gestor_avioes_t *gestor, aviao_t *aviao);
aviao_t *gestor_avioes_obter_por_id(gestor_avioes_t *gestor, const char *identificador);
unsigned gestor_avioes_contar(const gestor_avioes_t *gestor);
GHashTable *gestor_avioes_obter_tabela(gestor_avioes_t *gestor);
void gestor_avioes_para_cada(gestor_avioes_t *gestor, void (*func)(const char *, aviao_t *, void *), void *user_data);

// Função de carregamento (antes em _parser.h)
void gestor_avioes_carregar(gestor_avioes_t *gestor, const char *ficheiro_csv);

#endif