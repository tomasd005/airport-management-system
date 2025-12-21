#ifndef GESTOR_AEROPORTOS_H
#define GESTOR_AEROPORTOS_H

#include "../entidades/aeroportos.h"
#include <glib.h>

typedef struct gestor_aeroportos gestor_aeroportos_t;

gestor_aeroportos_t *gestor_aeroportos_criar(void);
void gestor_aeroportos_destruir(gestor_aeroportos_t *gestor);
void gestor_aeroportos_adicionar(gestor_aeroportos_t *gestor, aeroporto_t *aeroporto);
aeroporto_t *gestor_aeroportos_obter_por_codigo(gestor_aeroportos_t *gestor, const char *codigo);
unsigned int gestor_aeroportos_numero(gestor_aeroportos_t *gestor);
unsigned int gestor_aeroportos_contar(const gestor_aeroportos_t *gestor);
unsigned int gestor_aeroportos_total(const gestor_aeroportos_t *gestor);

// Função de carregamento (antes em _parser.h)
void gestor_aeroportos_carregar(gestor_aeroportos_t *gestor, const char *ficheiro_csv);

#endif