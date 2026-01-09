#ifndef QUERIE3_H
#define QUERIE3_H

#include <stdio.h>

/**
 * @file querie3.h
 * @brief Funções para a Query 3 do programa.
 *
 * Este módulo fornece a função para processar a Query 3, que envolve
 * aeroportos e voos num intervalo de datas, e escreve os resultados num ficheiro de saída.
 */

typedef struct gestor_aeroportos gestor_aeroportos_t;
typedef struct gestor_voos gestor_voos_t;

/**
 * @brief Executa a Query 3.
 * 
 * Processa a Query 3 com base nos dados de aeroportos e voos
 * dentro do intervalo de datas fornecido. Escreve os resultados
 * diretamente no ficheiro fornecido.
 * 
 * @param gestor_aeroportos Ponteiro para o gestor de aeroportos.
 * @param gestor_voos Ponteiro para o gestor de voos.
 * @param data_inicio Data de início do intervalo no formato "YYYY-MM-DD".
 * @param data_fim Data de fim do intervalo no formato "YYYY-MM-DD".
 * @param comando_completo String com o comando completo da query.
 * @param output Ponteiro para o ficheiro onde a saída será escrita.
 */
void query3(gestor_aeroportos_t *gestor_aeroportos,
            gestor_voos_t *gestor_voos,
            const char *data_inicio,
            const char *data_fim,
            const char *comando_completo, // NOVO parâmetro
            FILE *output);

#endif
