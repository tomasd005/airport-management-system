#include "validacao_comum.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include "utils.h"

#define ANO_ATUAL 2025
#define MES_ATUAL 9
#define DIA_ATUAL 30

gboolean contem_espacos(const char *str)
{
    if (!str)
        return FALSE;
    while (*str)
        if (isspace(*str++))
            return TRUE;
    return FALSE;
}

gboolean validacao_ano(const char *str, int *out_ano)
{
    if (!str || strlen(str) != 4 || contem_espacos(str))
        return FALSE;

    int ano = 0;
    for (int i = 0; i < 4; i++)
    {
        if (!isdigit(str[i]))
            return FALSE;
        ano = ano * 10 + (str[i] - '0');
    }

    if (ano < 1900 || ano > ANO_ATUAL)
        return FALSE;

    if (out_ano)
        *out_ano = ano;
    return TRUE;
}

gboolean validacao_data(const char *str)
{
    if (!str || strlen(str) != 10 || contem_espacos(str))
        return FALSE;

    if (str[4] != '-' || str[7] != '-')
        return FALSE;

    int ano = 0, mes = 0, dia = 0;

    for (int i = 0; i < 4; i++)
    {
        if (!isdigit(str[i]))
            return FALSE;
        ano = ano * 10 + (str[i] - '0');
    }

    for (int i = 5; i < 7; i++)
    {
        if (!isdigit(str[i]))
            return FALSE;
        mes = mes * 10 + (str[i] - '0');
    }

    for (int i = 8; i < 10; i++)
    {
        if (!isdigit(str[i]))
            return FALSE;
        dia = dia * 10 + (str[i] - '0');
    }

    return (ano >= 1900 && ano <= 2100 && mes >= 1 && mes <= 12 && dia >= 1 && dia <= 31);
}

gboolean validacao_data_passado(const char *str)
{
    if (!validacao_data(str))
        return FALSE;

    int ano = (str[0] - '0') * 1000 + (str[1] - '0') * 100 + (str[2] - '0') * 10 + (str[3] - '0');
    int mes = (str[5] - '0') * 10 + (str[6] - '0');
    int dia = (str[8] - '0') * 10 + (str[9] - '0');

    if (ano > ANO_ATUAL)
        return FALSE;
    if (ano == ANO_ATUAL && mes > MES_ATUAL)
        return FALSE;
    if (ano == ANO_ATUAL && mes == MES_ATUAL && dia > DIA_ATUAL)
        return FALSE;

    return TRUE;
}

gboolean validacao_datetime(const char *str)
{
    if (!str || strlen(str) != 16)
        return FALSE;

    if (str[4] != '-' || str[7] != '-' || str[10] != ' ' || str[13] != ':')
        return FALSE;

    if (strchr(str, '/'))
        return FALSE;

    for (int i = 0; i < 16; i++)
        if (i != 4 && i != 7 && i != 10 && i != 13 && !isdigit(str[i]))
            return FALSE;

    char data[11];
    memcpy(data, str, 10);
    data[10] = '\0';
    if (!validacao_data_passado(data))
        return FALSE;

    int h = (str[11] - '0') * 10 + (str[12] - '0');
    int m = (str[14] - '0') * 10 + (str[15] - '0');
    return (h <= 23 && m <= 59);
}

gboolean validacao_flight_id(const char *str)
{
    if (!str)
        return FALSE;

    char *temp = g_strdup(str);
    utils_trim(temp);

    if (contem_espacos(temp))
    {
        g_free(temp);
        return FALSE;
    }

    size_t len = strlen(temp);
    if (len < 7 || len > 9)
    {
        g_free(temp);
        return FALSE;
    }

    if (!isupper(temp[0]) || !isupper(temp[1]))
    {
        g_free(temp);
        return FALSE;
    }

    for (size_t i = 2; i < len; i++)
    {
        if (!isdigit(temp[i]))
        {
            g_free(temp);
            return FALSE;
        }
    }

    g_free(temp);
    return TRUE;
}

gboolean validacao_coordenada(const char *str, gboolean lat_mode, double *out_valor)
{
    if (!str || !*str || contem_espacos(str))
        return FALSE;

    int tem_ponto = 0, casas_decimais = 0;
    gboolean depois_ponto = FALSE;

    if (str[0] != '-' && !isdigit(str[0]))
        return FALSE;

    for (int i = (str[0] == '-') ? 1 : 0; str[i]; i++)
    {
        if (str[i] == '.')
        {
            if (++tem_ponto > 1)
                return FALSE;
            depois_ponto = TRUE;
        }
        else if (!isdigit(str[i]))
            return FALSE;
        else if (depois_ponto && ++casas_decimais > 8)
            return FALSE;
    }

    char *end;
    double val = strtod(str, &end);

    if (*end != '\0' || isnan(val) || isinf(val))
        return FALSE;

    if (lat_mode)
    {
        if (val < -90.0 || val > 90.0)
            return FALSE;
    }
    else
    {
        if (val < -180.0 || val > 180.0)
            return FALSE;
    }

    if (out_valor)
        *out_valor = val;
    return TRUE;
}

gboolean coordenadas_validas(const char *lat, const char *lon, double *out_lat, double *out_lon)
{
    return validacao_coordenada(lat, TRUE, out_lat) && validacao_coordenada(lon, FALSE, out_lon);
}

gboolean validacao_inteiro_positivo(const char *str, int *out_val)
{
    if (!str || !*str || contem_espacos(str))
        return FALSE;

    long val = 0;
    while (*str)
    {
        if (!isdigit(*str))
            return FALSE;
        val = val * 10 + (*str++ - '0');
    }

    if (val <= 0)
        return FALSE;
    if (out_val)
        *out_val = (int)val;
    return TRUE;
}

int comparar_datetime(const char *dt1, const char *dt2)
{
    if (!dt1 || !dt2)
        return 0;

    int y1, m1, d1, h1, min1;
    int y2, m2, d2, h2, min2;

    if (sscanf(dt1, "%d-%d-%d %d:%d", &y1, &m1, &d1, &h1, &min1) != 5)
        return 0;
    if (sscanf(dt2, "%d-%d-%d %d:%d", &y2, &m2, &d2, &h2, &min2) != 5)
        return 0;

    if (y1 != y2)
        return y1 - y2;
    if (m1 != m2)
        return m1 - m2;
    if (d1 != d2)
        return d1 - d2;
    if (h1 != h2)
        return h1 - h2;
    return min1 - min2;
}
