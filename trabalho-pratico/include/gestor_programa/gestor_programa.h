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

/**
 * @brief Cria um gestor de programa.
 * @param modoEconomiaMemoria TRUE para ativar modo de economia.
 * @return Gestor alocado, ou NULL em erro de memória.
 */
GestorDePrograma *gestor_programa_novo(gboolean modoEconomiaMemoria);

/**
 * @brief Executa o fluxo principal do programa.
 * @param gestor Gestor de programa.
 * @param pastaDados Pasta com os datasets.
 * @param ficheiroInput Ficheiro de comandos.
 */
void gestor_programa_executa(GestorDePrograma *gestor, const char *pastaDados, const char *ficheiroInput);

/**
 * @brief Destrói o gestor e liberta recursos globais.
 * @param gestor Gestor a destruir (aceita NULL).
 */
void gestor_programa_destroi(GestorDePrograma *gestor);

#endif
