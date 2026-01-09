#ifndef GESTOR_PROGRAMA_H
#define GESTOR_PROGRAMA_H

#include <glib.h>

/**
 * @file gestor_programa.h
 * @brief Interface do gestor principal do programa.
 *
 * Define a estrutura e funções para criar, executar e destruir
 * o gestor do programa, responsável por carregar os dados,
 * processar comandos e gerir a execução global.
 */

/**
 * @brief Estrutura opaca que representa o gestor do programa.
 */
typedef struct gestor_programa GestorDePrograma;

GestorDePrograma *gestor_programa_novo(gboolean modoEconomiaMemoria);
void gestor_programa_executa(GestorDePrograma *gestor, const char *pastaDados, const char *ficheiroInput);
void gestor_programa_destroi(GestorDePrograma *gestor);

#endif
