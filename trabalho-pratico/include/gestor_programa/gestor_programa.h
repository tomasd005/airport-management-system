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
 * @brief Cria um novo gestor do programa.
 *
 * @param modoEconomiaMemoria Flag para modo de economia de memória.
 * @return Ponteiro para o gestor criado.
 */
GestorDePrograma *gestor_programa_novo(gboolean modoEconomiaMemoria);

/**
 * @brief Executa o programa com os dados e comandos fornecidos.
 *
 * @param gestor Gestor do programa.
 * @param pastaDados Pasta do dataset.
 * @param ficheiroInput Ficheiro com comandos das queries.
 */
void gestor_programa_executa(GestorDePrograma *gestor, const char *pastaDados, const char *ficheiroInput);

/**
 * @brief Destroi o gestor do programa e liberta recursos.
 *
 * @param gestor Gestor do programa.
 */
void gestor_programa_destroi(GestorDePrograma *gestor);

#endif
