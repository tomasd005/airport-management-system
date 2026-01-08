#ifndef GESTOR_PROGRAMA_H
#define GESTOR_PROGRAMA_H

#include <glib.h>

typedef struct gestor_programa GestorDePrograma;

GestorDePrograma *gestor_programa_novo(gboolean modoEconomiaMemoria);
void gestor_programa_executa(GestorDePrograma *gestor, const char *pastaDados, const char *ficheiroInput);
void gestor_programa_destroi(GestorDePrograma *gestor);

#endif
