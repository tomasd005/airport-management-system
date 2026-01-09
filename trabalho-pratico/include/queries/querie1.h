#ifndef QUERY1_H
#define QUERY1_H

#include <stdio.h>

/**
 * @file query1.h
 * @brief Funções para a Query 1 do programa.
 *
 * Este módulo fornece a função para processar a Query 1, que envolve 
 * aeroportos, voos e reservas, e escreve os resultados num ficheiro de saída.
 */

typedef struct gestor_aeroportos gestor_aeroportos_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_reservas gestor_reservas_t;

/**
 * @brief Executa a Query 1.
 * 
 * Processa a Query 1 com base nos dados de aeroportos, voos e reservas.
 * Escreve a saída diretamente no ficheiro fornecido.
 * 
 * @param gestor_aeroportos Ponteiro para o gestor de aeroportos.
 * @param gestor_voos Ponteiro para o gestor de voos.
 * @param gestor_reservas Ponteiro para o gestor de reservas.
 * @param comando_completo String com o comando completo da query.
 * @param airport_code Código do aeroporto alvo da query.
 * @param output Ponteiro para o ficheiro onde a saída será escrita.
 */
void query1(gestor_aeroportos_t *gestor_aeroportos,
            gestor_voos_t *gestor_voos,
            gestor_reservas_t *gestor_reservas,
            const char *comando_completo,
            const char *airport_code,
            FILE *output);
#endif
