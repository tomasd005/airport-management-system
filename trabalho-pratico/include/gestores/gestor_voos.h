#ifndef GESTOR_VOOS_H
#define GESTOR_VOOS_H

#include <glib.h>
#include "../entidades/voos.h"

typedef struct gestor_voos gestor_voos_t;

gestor_voos_t *gestor_voos_criar(void);
void gestor_voos_destruir(gestor_voos_t *gestor);
void gestor_voos_adicionar(gestor_voos_t *gestor, voo_t *voo);
voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id);
unsigned gestor_voos_contar(const gestor_voos_t *gestor);
GHashTable *gestor_voos_obter_tabela(gestor_voos_t *gestor);
void gestor_voos_para_cada(gestor_voos_t *gestor, void (*func)(const char *, voo_t *, void *), void *user_data);

// Função de carregamento (antes em _parser.h)
void gestor_voos_carregar(gestor_voos_t *gestor, const char *ficheiro_csv);
// Adiciona estas declarações:
GPtrArray *gestor_voos_obter_por_origin(gestor_voos_t *gestor, const char *origin);
GPtrArray *gestor_voos_obter_por_destination(gestor_voos_t *gestor, const char *destination);
#endif
