#ifndef GESTOR_VOOS_H
#define GESTOR_VOOS_H

#include "../entidades/voos.h"
#include "../gestores/gestor_avioes.h"
#include <glib.h>

typedef struct gestor_voos gestor_voos_t;

gestor_voos_t *gestor_voos_criar(void);
void gestor_voos_destruir(gestor_voos_t *gestor);
void gestor_voos_adicionar(gestor_voos_t *gestor, voo_t *voo);
voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id);
unsigned int gestor_voos_contar(const gestor_voos_t *gestor);
const char *gestor_voos_obter_departure(gestor_voos_t *gestor, const char *flight_id);
void gestor_voos_carregar(gestor_voos_t *gestor, const char *ficheiro_csv);
void gestor_voos_carregar_com_validacao(gestor_voos_t *gestor, const char *ficheiro_csv, gestor_avioes_t *gestor_avioes);
void gestor_voos_para_cada(gestor_voos_t *gestor, void (*callback)(voo_t *, void *), void *user_data);
void gestor_voos_para_cada_origem(gestor_voos_t *gestor, const char *origin, void (*callback)(voo_t *, void *), void *user_data);
void gestor_voos_para_cada_destino(gestor_voos_t *gestor, const char *destination, void (*callback)(voo_t *, void *), void *user_data);

#endif
