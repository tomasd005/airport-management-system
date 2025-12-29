#include "validacao_comum.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define ANO_ATUAL 2025
#define MES_ATUAL 9
#define DIA_ATUAL 30

gboolean contem_espacos(const char *str)
{
    if (!str)
        return FALSE;
    for (int i = 0; str[i]; i++)
        if (isspace((unsigned char)str[i]))
            return TRUE;
    return FALSE;
}

gboolean validacao_ano(const char *str, int *out_ano)
{
    if (!str || strlen(str) != 4 || contem_espacos(str))
        return FALSE;
    for (int i = 0; i < 4; i++)
        if (!isdigit((unsigned char)str[i]))
            return FALSE;
    int ano = atoi(str);
    if (ano < 1900 || ano > ANO_ATUAL)
        return FALSE;
    if (out_ano)
        *out_ano = ano;
    return TRUE;
}

/* Validação de data BASE - não verifica se está no futuro */
gboolean validacao_data(const char *str)
{
    if (!str || strlen(str) != 10 || contem_espacos(str))
        return FALSE;

    if (str[4] != '-' || str[7] != '-')
        return FALSE;

    for (int i = 0; i < 10; i++)
        if (i != 4 && i != 7 && !isdigit((unsigned char)str[i]))
            return FALSE;

    int ano = (str[0] - '0') * 1000 + (str[1] - '0') * 100 + (str[2] - '0') * 10 + (str[3] - '0');
    int mes = (str[5] - '0') * 10 + (str[6] - '0');
    int dia = (str[8] - '0') * 10 + (str[9] - '0');

    if (ano < 1900 || ano > 2100 || mes < 1 || mes > 12 || dia < 1 || dia > 31)
        return FALSE;

    /* REMOVIDO: Validação de data futura
     * Essa validação deve ser feita apenas para datas de nascimento,
     * não para datas de voos (que podem ser futuras) */

    return TRUE;
}

/* Nova função para validar datas que devem estar no passado */
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
        if (i != 4 && i != 7 && i != 10 && i != 13 && !isdigit((unsigned char)str[i]))
            return FALSE;

    char data[11];
    memcpy(data, str, 10);
    data[10] = '\0';
    if (!validacao_data(data))
        return FALSE;

    int h = (str[11] - '0') * 10 + (str[12] - '0');
    int m = (str[14] - '0') * 10 + (str[15] - '0');
    return (h >= 0 && h <= 23 && m >= 0 && m <= 59);
}

gboolean validacao_flight_id(const char *str)
{
    if (!str || strlen(str) != 7 || contem_espacos(str))
        return FALSE;
    if (!isupper((unsigned char)str[0]) || !isupper((unsigned char)str[1]))
        return FALSE;
    for (int i = 2; i < 7; i++)
        if (!isdigit((unsigned char)str[i]))
            return FALSE;
    return TRUE;
}

gboolean validacao_coordenada(const char *str, gboolean lat_mode, double *out_valor)
{
    if (!str || !*str || contem_espacos(str))
        return FALSE;

    int tem_ponto = 0;
    int tem_sinal = 0;
    int num_digitos = 0;
    int casas_decimais = 0;
    gboolean depois_ponto = FALSE;

    for (int i = 0; str[i]; i++)
    {
        if (str[i] == '.')
        {
            tem_ponto++;
            if (tem_ponto > 1)
                return FALSE;
            depois_ponto = TRUE;
        }
        else if (str[i] == '-')
        {
            tem_sinal++;
            if (i != 0 || tem_sinal > 1)
                return FALSE;
        }
        else if (isdigit((unsigned char)str[i]))
        {
            num_digitos++;
            if (depois_ponto)
                casas_decimais++;
        }
        else
        {
            return FALSE;
        }
    }

    if (num_digitos == 0)
        return FALSE;

    if (casas_decimais > 8)
        return FALSE;

    char *end;
    double val = strtod(str, &end);

    if (*end != '\0')
        return FALSE;

    if (isnan(val) || isinf(val))
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
    return validacao_coordenada(lat, TRUE, out_lat) &&
           validacao_coordenada(lon, FALSE, out_lon);
}

gboolean validacao_inteiro_positivo(const char *str, int *out_val)
{
    if (!str || !*str || contem_espacos(str))
        return FALSE;
    for (int i = 0; str[i]; i++)
        if (!isdigit((unsigned char)str[i]))
            return FALSE;
    long val = strtol(str, NULL, 10);
    if (val <= 0)
        return FALSE;
    if (out_val)
        *out_val = (int)val;
    return TRUE;
}