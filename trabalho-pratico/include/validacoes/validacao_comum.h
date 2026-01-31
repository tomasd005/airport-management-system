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
 * @param str String a validar.
 * @return TRUE se contiver espaços, FALSE caso contrário.
 */
gboolean contem_espacos(const char *str);

/**
 * @brief Valida um ano (AAAA) e devolve o inteiro correspondente.
 * @param str String com o ano.
 * @param out_ano Saída com o ano convertido.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_ano(const char *str, int *out_ano);

/**
 * @brief Valida uma data no formato "AAAA-MM-DD".
 * @param str String com a data.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_data(const char *str);

/**
 * @brief Valida uma data e confirma que não é no futuro.
 * @param str String com a data.
 * @return TRUE se válido e no passado/presente, FALSE caso contrário.
 */
gboolean validacao_data_passado(const char *str);

/**
 * @brief Valida um datetime no formato "AAAA-MM-DD HH:MM".
 * @param str String com o datetime.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_datetime(const char *str);

/**
 * @brief Valida um identificador de voo (formato esperado).
 * @param str String com o ID.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_flight_id(const char *str);

/**
 * @brief Valida uma coordenada (latitude/longitude) e converte para double.
 * @param str String com a coordenada.
 * @param lat_mode TRUE para latitude, FALSE para longitude.
 * @param out_valor Saída com o valor convertido.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_coordenada(const char *str, gboolean lat_mode, double *out_valor);

/**
 * @brief Valida par de coordenadas e converte para double.
 * @param lat String com latitude.
 * @param lon String com longitude.
 * @param out_lat Saída com latitude convertida.
 * @param out_lon Saída com longitude convertida.
 * @return TRUE se válidas, FALSE caso contrário.
 */
gboolean coordenadas_validas(const char *lat, const char *lon, double *out_lat, double *out_lon);

/**
 * @brief Valida inteiro positivo e converte para int.
 * @param str String com o número.
 * @param out_val Saída com o inteiro convertido.
 * @return TRUE se válido, FALSE caso contrário.
 */
gboolean validacao_inteiro_positivo(const char *str, int *out_val);

/**
 * @brief Compara dois datetimes no formato "AAAA-MM-DD HH:MM".
 * @param dt1 Primeiro datetime.
 * @param dt2 Segundo datetime.
 * @return <0 se dt1 < dt2, 0 se dt1 == dt2, >0 se dt1 > dt2.
 */
int comparar_datetime(const char *dt1, const char *dt2);

#endif
