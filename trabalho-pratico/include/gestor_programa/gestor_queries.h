#ifndef GESTOR_QUERIES_H
#define GESTOR_QUERIES_H

#include "../gestores/gestor_aeroportos.h"
#include "../gestores/gestor_avioes.h"
#include "../gestores/gestor_voos.h"
#include "../gestores/gestor_passageiros.h"
#include "../gestores/gestor_reservas.h"

typedef struct gestor_queries gestor_queries_t;

gestor_queries_t *gestor_queries_criar(
    gestor_aeroportos_t *aeroportos,
    gestor_avioes_t *avioes,
    gestor_voos_t *voos,
    gestor_passageiros_t *passageiros,
    gestor_reservas_t *reservas);

void gestor_queries_destruir(gestor_queries_t *gestor);

void gestor_queries_processar_ficheiro(
    gestor_queries_t *gestor,
    const char *ficheiro_input);

#endif