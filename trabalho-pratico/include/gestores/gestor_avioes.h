#ifndef GESTOR_AVIOES_H
#define GESTOR_AVIOES_H

#include "../entidades/avioes.h"
#include <glib.h>

/**
 * @file gestor_avioes.h
 * @brief Gestão centralizada de aviões.
 *
 * Este módulo permite criar, destruir e gerir um conjunto de aviões,
 * incluindo inserção, consulta, carregamento a partir de CSV e iteração.
 */

typedef struct gestor_avioes gestor_avioes_t;

gestor_avioes_t *gestor_avioes_criar(void);
void gestor_avioes_destruir(gestor_avioes_t *gestor);
void gestor_avioes_adicionar(gestor_avioes_t *gestor, aviao_t *aviao);
aviao_t *gestor_avioes_obter_por_id(gestor_avioes_t *gestor, const char *identificador);
unsigned int gestor_avioes_contar(const gestor_avioes_t *gestor);
void gestor_avioes_carregar(gestor_avioes_t *gestor, const char *ficheiro_csv);
void gestor_avioes_para_cada(gestor_avioes_t *gestor, void (*callback)(aviao_t *, void *), void *user_data);

#endif
