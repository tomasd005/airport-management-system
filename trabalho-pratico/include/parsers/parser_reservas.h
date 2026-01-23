#ifndef PARSER_RESERVAS_H
#define PARSER_RESERVAS_H

#include <glib.h>

/**
 * @file parser_reservas.h
 * @brief Parser especializado para reservas (CSV).
 */

/**
 * @brief Callback de processamento de uma linha de reservas.
 *
 * @param contexto Contexto do utilizador.
 * @param colunas Colunas da linha (strings mutáveis).
 * @return TRUE se a linha foi processada com sucesso, FALSE caso contrário.
 */
typedef gboolean (*ReservaProcessaLinha)(void *contexto, char **colunas);

/**
 * @brief Carrega um CSV de reservas e invoca o callback por linha válida.
 *
 * Para datasets "sem_erros" usa mmap quando o ficheiro é grande.
 */
void parser_reservas_carregar(void *contexto,
                              const char *ficheiro_csv,
                              ReservaProcessaLinha processa_linha);

#endif
