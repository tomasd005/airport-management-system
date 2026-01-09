#ifndef QUERIE2_H
#define QUERIE2_H

#include <stdio.h>

/**
 * @file querie2.h
 * @brief Funções para a Query 2 do programa.
 *
 * Este módulo fornece a função para processar a Query 2, que envolve
 * aviões e voos, e escreve os resultados num ficheiro de saída.
 */

typedef struct gestor_avioes gestor_avioes_t;
typedef struct gestor_voos gestor_voos_t;

/**
 * @brief Executa a Query 2.
 * 
 * Processa a Query 2 com base nos dados de aviões e voos.
 * Pode filtrar resultados por fabricante e limita a saída aos N registos.
 * Escreve a saída diretamente no ficheiro fornecido.
 * 
 * @param gestor_avioes Ponteiro para o gestor de aviões.
 * @param gestor_voos Ponteiro para o gestor de voos.
 * @param N Número máximo de registos a retornar.
 * @param fabricante Nome do fabricante para filtrar (ou NULL para todos).
 * @param comando_completo String com o comando completo da query.
 * @param output Ponteiro para o ficheiro onde a saída será escrita.
 */
void query2(gestor_avioes_t *gestor_avioes,
            gestor_voos_t *gestor_voos,
            int N,
            const char *fabricante,
            const char *comando_completo, // NOVO parâmetro
            FILE *output);
#endif
