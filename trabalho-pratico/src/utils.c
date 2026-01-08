#include "utils.h"
#include <string.h>
#include <glib.h>
#include <ctype.h>

void utils_remove_aspas_somente(char *str)
{
    if (!str || !*str)
        return;

    size_t len = strlen(str);
    size_t i = 0, j = 0;

    while (i < len && (str[i] == '"' || str[i] == '\''))
        i++;

    while (i < len)
    {
        if (str[i] == '"' || str[i] == '\'')
        {
            size_t k = i + 1;
            while (k < len && (str[k] == '"' || str[k] == '\'' || isspace((unsigned char)str[k])))
                k++;

            if (k == len)
                break;
        }

        str[j++] = str[i++];
    }
    str[j] = '\0';
}

void utils_remove_aspas(char *str)
{
    if (!str || !*str)
        return;

    size_t len = strlen(str);
    size_t i = 0, j = 0;

    while (i < len && isspace((unsigned char)str[i]))
        i++;

    while (i < len && (str[i] == '"' || str[i] == '\''))
        i++;

    while (i < len)
    {
        if ((str[i] == '"' || str[i] == '\''))
        {
            size_t k = i + 1;
            while (k < len && isspace((unsigned char)str[k]))
                k++;
            if (k == len)
                break;
        }
        str[j++] = str[i++];
    }
    str[j] = '\0';

    while (j > 0 && isspace((unsigned char)str[j - 1]))
        str[--j] = '\0';
}

void utils_remove_newline(char *str)
{
    if (!str)
        return;

    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r'))
    {
        str[len - 1] = '\0';
        len--;
    }
}

char *utils_obtem_nome_ficheiro(const char *caminho)
{
    if (!caminho)
        return g_strdup("desconhecido");

    const char *nome = strrchr(caminho, '/');
    if (!nome)
        nome = caminho;
    else
        nome++;

    char *base = g_strdup(nome);
    char *ponto = strrchr(base, '.');
    if (ponto)
        *ponto = '\0';

    return base;
}

void utils_trim(char *s)
{
    if (!s)
        return;

    char *p = s;
    while (isspace((unsigned char)*p))
        p++;

    if (p != s)
        memmove(s, p, strlen(p) + 1);

    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end))
        *end-- = '\0';
}

static int days_from_civil(int y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (int)doe - 719468;
}

static int parse_date_components(const char *date, int *y, int *m, int *d)
{
    if (!date || strlen(date) < 10)
        return 0;
    if (date[4] != '-' || date[7] != '-')
        return 0;
    if (!isdigit((unsigned char)date[0]) || !isdigit((unsigned char)date[1]) ||
        !isdigit((unsigned char)date[2]) || !isdigit((unsigned char)date[3]) ||
        !isdigit((unsigned char)date[5]) || !isdigit((unsigned char)date[6]) ||
        !isdigit((unsigned char)date[8]) || !isdigit((unsigned char)date[9]))
        return 0;

    *y = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + (date[3] - '0');
    *m = (date[5] - '0') * 10 + (date[6] - '0');
    *d = (date[8] - '0') * 10 + (date[9] - '0');
    return 1;
}

int utils_parse_date_to_day(const char *date)
{
    int y, m, d;
    if (!parse_date_components(date, &y, &m, &d))
        return -1;
    return days_from_civil(y, (unsigned)m, (unsigned)d);
}

int utils_parse_datetime_to_day(const char *datetime)
{
    if (!datetime || strcmp(datetime, "N/A") == 0 || strlen(datetime) < 16)
        return -1;
    return utils_parse_date_to_day(datetime);
}

int utils_parse_datetime_to_minutes(const char *datetime)
{
    if (!datetime || strcmp(datetime, "N/A") == 0 || strlen(datetime) < 16)
        return -1;
    if (datetime[10] != ' ' || datetime[13] != ':')
        return -1;
    if (!isdigit((unsigned char)datetime[11]) || !isdigit((unsigned char)datetime[12]) ||
        !isdigit((unsigned char)datetime[14]) || !isdigit((unsigned char)datetime[15]))
        return -1;

    int day = utils_parse_date_to_day(datetime);
    if (day < 0)
        return -1;

    int h = (datetime[11] - '0') * 10 + (datetime[12] - '0');
    int min = (datetime[14] - '0') * 10 + (datetime[15] - '0');
    return day * 1440 + h * 60 + min;
}

int utils_week_from_day(int day)
{
    if (day < 0)
        return -1;
    int wday = (day + 4) % 7;
    if (wday < 0)
        wday += 7;
    return (day - wday) / 7;
}
