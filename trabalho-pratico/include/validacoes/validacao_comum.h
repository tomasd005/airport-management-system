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

gboolean contem_espacos(const char *str);
gboolean validacao_ano(const char *str, int *out_ano);
gboolean validacao_data(const char *str);
gboolean validacao_data_passado(const char *str); /* Nova função */
gboolean validacao_datetime(const char *str);
gboolean validacao_flight_id(const char *str);
gboolean validacao_coordenada(const char *str, gboolean lat_mode, double *out_valor);
gboolean coordenadas_validas(const char *lat, const char *lon, double *out_lat, double *out_lon);
gboolean validacao_inteiro_positivo(const char *str, int *out_val);

// Compara dois datetimes no formato "AAAA-MM-DD HH:MM"
// Retorna: <0 se dt1 < dt2, 0 se dt1 == dt2, >0 se dt1 > dt2
int comparar_datetime(const char *dt1, const char *dt2);

#endif