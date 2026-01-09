#ifndef QUERIE6_H
#define QUERIE6_H

#include <stdio.h>

/**
 * @file querie6.h
 * @brief Funções para a Query 6 do programa.
 *
 * Este módulo fornece a função para processar a Query 6, que envolve
 * a análise de reservas, voos e passageiros, filtrando por nacionalidade
 * e produzindo os resultados correspondentes.
 */

typedef struct gestor_reservas gestor_reservas_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_passageiros gestor_passageiros_t;

/**
 * @brief Executa a Query 6.
 * 
 * Processa a Query 6 usando os gestores de reservas, voos e passageiros,
 * filtrando os passageiros por nacionalidade e gerando os resultados
 * correspondentes. Os resultados são escritos no ficheiro de saída.
 * 
 * @param gestor_reservas Ponteiro para o gestor de reservas.
 * @param gestor_voos Ponteiro para o gestor de voos.
 * @param gestor_passageiros Ponteiro para o gestor de passageiros.
 * @param nacionalidade String com a nacionalidade a filtrar.
 * @param comando_completo String com o comando completo da query.
 * @param output Ponteiro para o ficheiro onde a saída será escrita.
 */
void query6(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *nacionalidade,
            const char *comando_completo,
            FILE *output);

#endif
