#ifndef GESTOR_QUERIES_H
#define GESTOR_QUERIES_H

typedef struct gestor_queries gestor_queries_t;
typedef struct gestor_aeroportos gestor_aeroportos_t;
typedef struct gestor_avioes gestor_avioes_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_passageiros gestor_passageiros_t;
typedef struct gestor_reservas gestor_reservas_t;

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
