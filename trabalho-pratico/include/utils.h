#ifndef UTILS_H
#define UTILS_H

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
 * @brief Converte uma datetime no formato "YYYY-MM-DD HH:MM" para número de minutos desde 01/01/1900 00:00.
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

#endif
