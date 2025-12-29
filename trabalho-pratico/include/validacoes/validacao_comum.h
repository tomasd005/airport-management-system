#ifndef VALIDACAO_COMUM_H
#define VALIDACAO_COMUM_H

#include <glib.h>

gboolean contem_espacos(const char *str);
gboolean validacao_ano(const char *str, int *out_ano);
gboolean validacao_data(const char *str);
gboolean validacao_data_passado(const char *str); /* Nova função */
gboolean validacao_datetime(const char *str);
gboolean validacao_flight_id(const char *str);
gboolean validacao_coordenada(const char *str, gboolean lat_mode, double *out_valor);
gboolean coordenadas_validas(const char *lat, const char *lon, double *out_lat, double *out_lon);
gboolean validacao_inteiro_positivo(const char *str, int *out_val);

#endif