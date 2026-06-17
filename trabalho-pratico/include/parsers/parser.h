#ifndef PARSER_H
#define PARSER_H

#include <glib.h>
#include <time.h>

/**
 * @file parser.h
 * @brief Funções genéricas para parsing de CSV e conversão de datas.
 *
 * Este módulo fornece ferramentas para dividir linhas CSV em colunas,
 * criar objetos a partir de linhas, adicionar objetos a coleções,
 * destruir objetos e converter strings datetime para time_t.
 */

/**
 * @brief Função que converte uma linha de CSV em um objeto.
 *
 * @param colunas Vetor de strings contendo os campos da linha.
 * @return Ponteiro genérico para o objeto criado.
 */
typedef gpointer (*LinhaParaObjeto)(char **colunas);

/**
 * @brief Função que adiciona um objeto a uma coleção/contexto.
 *
 * @param contexto Contexto ou estrutura que irá armazenar o objeto.
 * @param objeto Ponteiro para o objeto a adicionar.
 * @return TRUE se a adição foi bem-sucedida, FALSE caso contrário.
 */
typedef gboolean (*AdicionaObjeto)(void *contexto, gpointer objeto);

/**
 * @brief Função que destrói um objeto.
 *
 * @param objeto Ponteiro para o objeto a destruir.
 */
typedef void (*DestroiObjeto)(gpointer objeto);

/**
 * @brief Divide uma linha CSV em colunas.
 *
 * Esta função separa uma linha CSV em tokens, removendo aspas se necessário.
 *
 * @param linha Linha CSV a dividir.
 * @param colunas Vetor de strings para armazenar cada coluna.
 * @param max_colunas Número máximo de colunas a processar.
 * @return Número de colunas efetivamente preenchidas.
 */
int parser_dividir_csv(char *linha, char **colunas, int max_colunas);

/**
 * @brief Divide uma linha CSV até um número mínimo de colunas necessárias.
 *
 * Para quando atinge colunas_necessarias.
 *
 * @param linha Linha CSV a dividir.
 * @param colunas Vetor de strings para armazenar cada coluna.
 * @param max_colunas Número máximo de colunas.
 * @param colunas_necessarias Número mínimo de colunas a preencher.
 * @return Número de colunas efetivamente preenchidas.
 */
int parser_dividir_csv_ate(char *linha, char **colunas, int max_colunas, int colunas_necessarias);

/**
 * @brief Indica se o parser está a usar mmap.
 *
 * @return 1 se mmap está ativo, 0 caso contrário.
 */
int parser_mmap_em_uso(void);

/**
 * @brief Define se o parser está no caminho mmap.
 * @param ativo 1 para ativar, 0 para desativar.
 */
void parser_definir_mmap_em_uso(int ativo);

/**
 * @brief Indica se o parser está em modo sem_erros.
 * @return 1 se está a processar dataset sem_erros, 0 caso contrário.
 */
int parser_sem_erros_ativo(void);

/**
 * @brief Define o estado de processamento em modo sem_erros.
 * @param ativo 1 para ativar, 0 para desativar.
 */
void parser_definir_sem_erros(int ativo);

/**
 * @brief Indica se o parser está a processar um dataset grande.
 * @return 1 se está a processar um dataset grande, 0 caso contrário.
 */
int parser_dataset_grande_ativo(void);

/**
 * @brief Define o estado de processamento de dataset grande.
 * @param ativo 1 para ativar, 0 para desativar.
 */
void parser_definir_dataset_grande(int ativo);

/**
 * @brief Carrega um ficheiro CSV, converte linhas em objetos e adiciona-os a um contexto.
 *
 * @param contexto Estrutura de armazenamento ou gestor onde os objetos serão adicionados.
 * @param ficheiro_csv Caminho para o ficheiro CSV.
 * @param adiciona_objeto Função callback para adicionar objetos ao contexto.
 * @param linha_para_objeto Função callback para converter uma linha CSV em um objeto.
 * @param destroi_objeto Função callback para destruir objetos em caso de falha.
 */
void parser_carrega(void *contexto, const char *ficheiro_csv, AdicionaObjeto adiciona_objeto,
                    LinhaParaObjeto linha_para_objeto, DestroiObjeto destroi_objeto,
                    int max_colunas, int colunas_necessarias);

/**
 * @brief Converte uma string datetime (formato "YYYY-MM-DD HH:MM") para time_t.
 *
 * @param datetime String com a data e hora.
 * @return Valor time_t correspondente à datetime, ou (time_t)-1 em caso de erro.
 */
time_t parser_datetime_para_time(const char *datetime);

#endif
