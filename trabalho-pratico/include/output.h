#ifndef OUTPUT_H
#define OUTPUT_H

/**
 * @file output.h
 * @brief Funções utilitárias para criação e escrita de arquivos de saída.
 *
 * Este módulo fornece funções para criar arquivos de saída por comando,
 * escrever linhas e fechar arquivos.
 */

#include <stdio.h>
#include <glib.h>

/**
 * @brief Cria um arquivo de saída para um comando específico.
 *
 * O arquivo é criado com um nome padrão baseado no número do comando.
 *
 * @param command_number Número do comando que gerou o arquivo.
 * @return Ponteiro para o arquivo aberto em escrita, ou NULL em caso de erro.
 */
FILE *create_output_file(int command_number);

/**
 * @brief Escreve uma linha em um arquivo de saída.
 *
 * Adiciona uma quebra de linha automaticamente ao final da string.
 *
 * @param f Ponteiro para o arquivo de saída.
 * @param line Linha de texto a ser escrita.
 */
void write_line(FILE *f, const char *line);

/**
 * @brief Fecha um arquivo de saída previamente aberto.
 *
 * Garante que o arquivo seja fechado corretamente.
 *
 * @param f Ponteiro para o arquivo a ser fechado.
 */
void close_output_file(FILE *f);

#endif
