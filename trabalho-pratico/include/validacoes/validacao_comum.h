#ifndef VALIDACAO_COMUM_H
#define VALIDACAO_COMUM_H

#include <glib.h>

/**
 * @file validacao_comum.h
 * @brief Funções comuns de validação de dados para aeroportos, voos, reservas, etc.
 *
 * Este módulo contém funções genéricas que validam formatos de strings,
 * datas, horários, códigos de voo, coordenadas e números inteiros positivos.
 */

/**
 * @brief Verifica se a string contém espaços.
 *
 * @param str String a validar.
 * @return TRUE se contiver espaços, FALSE caso contrário.
 */
gboolean contem_espacos(const char *str);

/**
 * @brief Valida um ano no formato numérico.
 *
 * @param str String com o ano.
 * @param out_ano Output do ano convertido.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_ano(const char *str, int *out_ano);

/**
 * @brief Valida uma data no formato YYYY-MM-DD.
 *
 * @param str String da data.
 * @return TRUE se válida, FALSE caso contrário.
 */
gboolean validacao_data(const char *str);

/**
 * @brief Valida uma data no passado (YYYY-MM-DD).
 *
 * @param str String da data.
 * @return TRUE se válida e no passado, FALSE caso contrário.
 */
gboolean validacao_data_passado(const char *str);

/**
 * @brief Valida um datetime no formato YYYY-MM-DD HH:MM.
 *
 * @param str String do datetime.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_datetime(const char *str);

/**
 * @brief Valida um identificador de voo.
 *
 * @param str String do flight id.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_flight_id(const char *str);

/**
 * @brief Valida uma coordenada (latitude/longitude).
 *
 * @param str String da coordenada.
 * @param lat_mode TRUE para latitude, FALSE para longitude.
 * @param out_valor Output do valor convertido.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_coordenada(const char *str, gboolean lat_mode, double *out_valor);

/**
 * @brief Valida um par de coordenadas latitude/longitude.
 *
 * @param lat String da latitude.
 * @param lon String da longitude.
 * @param out_lat Output da latitude convertida.
 * @param out_lon Output da longitude convertida.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean coordenadas_validas(const char *lat, const char *lon, double *out_lat, double *out_lon);

/**
 * @brief Valida um inteiro positivo e devolve o valor.
 *
 * @param str String do número.
 * @param out_val Output do valor convertido.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_inteiro_positivo(const char *str, int *out_val);

/**
 * @brief Compara dois datetimes no formato "AAAA-MM-DD HH:MM".
 *
 * @param dt1 Primeiro datetime.
 * @param dt2 Segundo datetime.
 * @return <0 se dt1 < dt2, 0 se dt1 == dt2, >0 se dt1 > dt2.
 */
int comparar_datetime(const char *dt1, const char *dt2);

#endif
