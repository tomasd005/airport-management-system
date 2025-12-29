#ifndef PARSER_H
#define PARSER_H

#include <glib.h>
#include <time.h>

typedef gpointer (*LinhaParaObjeto)(char **colunas);

typedef gboolean (*AdicionaObjeto)(void *contexto, gpointer objeto);

typedef void (*DestroiObjeto)(gpointer objeto);

void parser_carrega(void *contexto,
                    const char *ficheiro_csv,
                    AdicionaObjeto adiciona_objeto,
                    LinhaParaObjeto linha_para_objeto,
                    DestroiObjeto destroi_objeto);


time_t parser_datetime_para_time(const char *datetime);

#endif
