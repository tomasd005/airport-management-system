#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * @file utils.h
 * @brief Funções utilitárias para manipulação de strings, datas e caminhos de ficheiros.
 *
 * Este módulo fornece funções auxiliares usadas em várias partes do programa,
 * incluindo remoção de aspas, trims, manipulação de datas e extração de nomes de ficheiro.
 */

/**
 * @brief Remove apenas aspas simples e duplas de uma string.
 *
 * As aspas são removidas mas outros caracteres permanecem intactos.
 *
 * @param str String a ser modificada.
 */
void utils_remove_aspas_somente(char *str);

/**
 * @brief Remove aspas simples e duplas de uma string.
 *
 * Pode ser usada para limpar campos de CSV antes de validação.
 *
 * @param str String a ser modificada.
 */
void utils_remove_aspas(char *str);

/**
 * @brief Remove o caractere de newline (\n) do final da string, se existir.
 *
 * @param str String a ser modificada.
 */
void utils_remove_newline(char *str);

/**
 * @brief Obtém o nome do ficheiro a partir de um caminho completo.
 *
 * Exemplo: "/home/user/file.txt" -> "file.txt"
 *
 * @param caminho Caminho completo.
 * @return Ponteiro para o nome do ficheiro (internamente alocado).
 */
char *utils_obtem_nome_ficheiro(const char *caminho);

/**
 * @brief Remove espaços em branco do início e do fim da string.
 *
 * @param s String a ser modificada.
 */
void utils_trim(char *s);

/**
 * @brief Converte uma data no formato "YYYY-MM-DD" para número de dias desde 01/01/1900.
 *
 * @param date String da data.
 * @return Número de dias desde 01/01/1900.
 */
int utils_parse_date_to_day(const char *date);

/**
 * @brief Converte uma datetime no formato "YYYY-MM-DD HH:MM" para número de dias desde 01/01/1900.
 *
 * @param datetime String da datetime.
 * @return Número de dias desde 01/01/1900.
 */
int utils_parse_datetime_to_day(const char *datetime);

/**
 * @brief Converte uma datetime no formato "YYYY-MM-DD HH:MM" para número de minutos desde
 * 01/01/1900 00:00.
 *
 * @param datetime String da datetime.
 * @return Número de minutos desde 01/01/1900 00:00.
 */
int utils_parse_datetime_to_minutes(const char *datetime);

/**
 * @brief Retorna a semana do ano a partir do número de dias desde 01/01/1900.
 *
 * @param day Número de dias desde 01/01/1900.
 * @return Número da semana.
 */
int utils_week_from_day(int day);

/**
 * @brief Versão rápida do parse de datetime para dias.
 *
 * Assume formato YYYY-MM-DD HH:MM e evita validações custosas.
 *
 * @param datetime String da datetime.
 * @return Número de dias desde 01/01/1900 ou -1 se inválido.
 */
int utils_parse_datetime_to_day_fast(const char *datetime);

/**
 * @brief Versão rápida do parse de datetime para minutos.
 *
 * @param datetime String da datetime.
 * @return Minutos desde 01/01/1900 00:00 ou -1 se inválido.
 */
int utils_parse_datetime_to_minutes_fast(const char *datetime);

/**
 * @brief Converte um código IATA (3 letras) num índice [0..17575].
 *
 * @param code Código IATA (ex: \"LIS\").
 * @return Índice numérico ou -1 se inválido.
 */
int utils_aeroporto_index(const char *code);

/**
 * @brief Converte document number (9 dígitos) para chave numérica.
 *
 * @param doc String com 9 dígitos.
 * @param out_key Output da chave numérica.
 * @return 1 se válido, 0 caso contrário.
 */
int utils_document_number_key(const char *doc, uint32_t *out_key);

/**
 * @brief Converte flight id (2 letras + 4..7 dígitos) para chave numérica.
 *
 * @param id Identificador do voo.
 * @param out_key Output da chave numérica.
 * @return 1 se válido, 0 caso contrário.
 */
int utils_flight_id_key(const char *id, uint64_t *out_key);

/**
 * @brief Converte índice de aeroporto [0..17575] em código IATA (3 letras).
 *
 * @param idx Índice numérico.
 * @param out Buffer de 4 bytes (3 letras + terminador).
 */
void utils_aeroporto_codigo(int idx, char out[4]);

/**
 * @brief Obtém o código IATA (3 letras) a partir de um índice.
 *
 * Retorna um ponteiro para um buffer interno estático.
 *
 * @param idx Índice numérico.
 * @return Ponteiro para string com 3 letras ou NULL se inválido.
 */
const char *utils_aeroporto_codigo_const(int idx);

#endif
