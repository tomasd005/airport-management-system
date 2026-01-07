#ifndef AVIOES_H
#define AVIOES_H

#include <stdbool.h>

typedef struct aviao aviao_t;

aviao_t *aviao_criar(const char *identificador, const char *fabricante, const char *modelo, int ano, int capacidade, int alcance_km);

void aviao_destruir(aviao_t *a);

const char *aviao_obter_identificador(const aviao_t *a);
const char *aviao_obter_fabricante(const aviao_t *a);
const char *aviao_obter_modelo(const aviao_t *a);
int aviao_obter_ano(const aviao_t *a);
int aviao_obter_capacidade(const aviao_t *a);
int aviao_obter_alcance_km(const aviao_t *a);
int aviao_obter_contagem_voos(const aviao_t *a);
void aviao_incrementar_contagem_voos(aviao_t *a, int delta);

#endif
