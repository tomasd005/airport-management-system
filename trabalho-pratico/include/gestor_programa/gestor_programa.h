#ifndef GESTOR_PROGRAMA_H
#define GESTOR_PROGRAMA_H

#include <glib.h>
#include "gestor_aeroportos.h"
#include "gestor_avioes.h"
#include "gestor_voos.h"
#include "gestor_passageiros.h"
#include "gestor_reservas.h"

typedef struct gestor_programa
{
    gestor_aeroportos_t *aeroportos;
    gestor_avioes_t *avioes;
    gestor_voos_t *voos;
    gestor_passageiros_t *passageiros;
    gestor_reservas_t *reservas;
    gboolean modoEconomiaMemoria;
} GestorDePrograma;

GestorDePrograma *gestor_programa_novo(gboolean modoEconomiaMemoria);
void gestor_programa_executa(GestorDePrograma *gestor, const char *pastaDados, const char *ficheiroInput);
void gestor_programa_destroi(GestorDePrograma *gestor);

#endif
