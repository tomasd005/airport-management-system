#ifndef GESTOR_RESERVAS_H
#define GESTOR_RESERVAS_H

#include <glib.h>

typedef struct gestor_reservas gestor_reservas_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_passageiros gestor_passageiros_t;

gestor_reservas_t *gestor_reservas_criar(void);
void gestor_reservas_destruir(gestor_reservas_t *gestor);
unsigned int gestor_reservas_numero(gestor_reservas_t *gestor);
void gestor_reservas_carregar_com_validacao(gestor_reservas_t *gestor, const char *ficheiro_csv, gestor_voos_t *gestor_voos, gestor_passageiros_t *gestor_passageiros);
void gestor_reservas_finalizar(gestor_reservas_t *gestor);
const GPtrArray *gestor_reservas_obter_top10_semana(gestor_reservas_t *gestor, int semana);
gboolean gestor_reservas_obter_melhor_destino_nacionalidade(gestor_reservas_t *gestor, const char *nac, const char **destino, guint *count);
void gestor_reservas_para_cada_top10(gestor_reservas_t *gestor, void (*callback)(int semana, const GPtrArray *top10, void *user_data), void *user_data);

#endif
