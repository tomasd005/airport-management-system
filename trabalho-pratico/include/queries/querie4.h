#ifndef QUERIE4_H
#define QUERIE4_H

#include <stdio.h>

/**
 * @file querie4.h
 * @brief Funções para a Query 4 do programa.
 *
 * Este módulo fornece a função para processar a Query 4, que envolve
 * reservas, voos e passageiros num intervalo de datas, e escreve os resultados
 * num ficheiro de saída.
 */

typedef struct gestor_reservas gestor_reservas_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_passageiros gestor_passageiros_t;

/**
 * @brief Executa a Query 4.
 * 
 * Processa a Query 4 usando os gestores de reservas, voos e passageiros
 * dentro do intervalo de datas fornecido. Os resultados são escritos
 * diretamente no ficheiro fornecido.
 * 
 * @param gestor_reservas Ponteiro para o gestor de reservas.
 * @param gestor_voos Ponteiro para o gestor de voos.
 * @param gestor_passageiros Ponteiro para o gestor de passageiros.
 * @param data_inicio Data de início do intervalo no formato "YYYY-MM-DD".
 * @param data_fim Data de fim do intervalo no formato "YYYY-MM-DD".
 * @param comando_completo String com o comando completo da query.
 * @param output Ponteiro para o ficheiro onde a saída será escrita.
 */
void query4(gestor_reservas_t *gestor_reservas,
            gestor_voos_t *gestor_voos,
            gestor_passageiros_t *gestor_passageiros,
            const char *data_inicio,
            const char *data_fim,
            const char *comando_completo,
            FILE *output);

#endif
