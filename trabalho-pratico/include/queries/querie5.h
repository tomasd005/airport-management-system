#ifndef QUERIE5_H
#define QUERIE5_H

#include <stdio.h>

/**
 * @file querie5.h
 * @brief Funções para a Query 5 do programa.
 *
 * Este módulo fornece a função para processar a Query 5, que envolve
 * a análise de voos e cálculos de atraso médio por companhia aérea,
 * retornando os N primeiros resultados.
 */

typedef struct gestor_voos gestor_voos_t;

/**
 * @brief Executa a Query 5.
 * 
 * Processa a Query 5 usando o gestor de voos, determinando os N primeiros
 * resultados relevantes de acordo com os critérios da query, e escreve os
 * resultados no ficheiro de saída.
 * 
 * @param gestor_voos Ponteiro para o gestor de voos.
 * @param N Número de resultados a retornar.
 * @param comando_completo String com o comando completo da query.
 * @param output Ponteiro para o ficheiro onde a saída será escrita.
 */
void query5(
    gestor_voos_t *gestor_voos,
    int N,
    const char *comando_completo,
    FILE *output);

#endif
