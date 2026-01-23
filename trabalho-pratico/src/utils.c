#include "utils.h"
#include <string.h>
#include <glib.h>
#include <ctype.h>
#include <stdint.h>

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

int utils_parse_datetime_to_day_fast(const char *datetime)
{
    if (!datetime || datetime[0] == 'N')
        return -1;
    if (datetime[4] != '-' || datetime[7] != '-' || datetime[10] != ' ')
        return -1;

    int y = (datetime[0] - '0') * 1000 + (datetime[1] - '0') * 100 +
            (datetime[2] - '0') * 10 + (datetime[3] - '0');
    int m = (datetime[5] - '0') * 10 + (datetime[6] - '0');
    int d = (datetime[8] - '0') * 10 + (datetime[9] - '0');

    if (y < 0 || m <= 0 || m > 12 || d <= 0 || d > 31)
        return -1;

    return days_from_civil(y, (unsigned)m, (unsigned)d);
}

int utils_parse_datetime_to_minutes_fast(const char *datetime)
{
    if (!datetime || datetime[0] == 'N')
        return -1;
    if (datetime[10] != ' ' || datetime[13] != ':')
        return -1;

    int day = utils_parse_datetime_to_day_fast(datetime);
    if (day < 0)
        return -1;

    int h = (datetime[11] - '0') * 10 + (datetime[12] - '0');
    int min = (datetime[14] - '0') * 10 + (datetime[15] - '0');
    if (h < 0 || h > 23 || min < 0 || min > 59)
        return -1;
    return day * 1440 + h * 60 + min;
}

int utils_aeroporto_index(const char *code)
{
    if (!code)
        return -1;
    if (strlen(code) != 3)
        return -1;
    char a = code[0];
    char b = code[1];
    char c = code[2];
    if (a < 'A' || a > 'Z' || b < 'A' || b > 'Z' || c < 'A' || c > 'Z')
        return -1;
    return (a - 'A') * 26 * 26 + (b - 'A') * 26 + (c - 'A');
}

int utils_document_number_key(const char *doc, uint32_t *out_key)
{
    if (!doc || !out_key)
        return 0;

    uint32_t value = 0;
    for (int i = 0; i < 9; i++)
    {
        unsigned char c = (unsigned char)doc[i];
        if (c < '0' || c > '9')
            return 0;
        value = value * 10u + (uint32_t)(c - '0');
    }
    if (doc[9] != '\0')
        return 0;

    *out_key = value;
    return 1;
}

int utils_flight_id_key(const char *id, uint64_t *out_key)
{
    if (!id || !out_key)
        return 0;
    if (id[0] < 'A' || id[0] > 'Z' || id[1] < 'A' || id[1] > 'Z')
        return 0;

    uint32_t num = 0;
    int digits = 0;
    for (const char *p = id + 2; *p; p++)
    {
        if (!isdigit((unsigned char)*p))
            return 0;
        num = num * 10u + (uint32_t)(*p - '0');
        digits++;
    }

    if (digits < 4 || digits > 7)
        return 0;

    uint32_t letters = (uint32_t)(id[0] - 'A') * 26u + (uint32_t)(id[1] - 'A');
    *out_key = ((uint64_t)letters * 100000000ull) + ((uint64_t)digits * 10000000ull) + num;
    return 1;
}

void utils_aeroporto_codigo(int idx, char out[4])
{
    if (!out)
        return;
    if (idx < 0 || idx >= (26 * 26 * 26))
    {
        out[0] = '\0';
        return;
    }

    int a = idx / (26 * 26);
    int b = (idx / 26) % 26;
    int c = idx % 26;
    out[0] = (char)('A' + a);
    out[1] = (char)('A' + b);
    out[2] = (char)('A' + c);
    out[3] = '\0';
}

const char *utils_aeroporto_codigo_const(int idx)
{
    enum { NUM_AEROPORTOS = 26 * 26 * 26 };
    static char codes[NUM_AEROPORTOS][4];
    static int initialized = 0;

    if (idx < 0 || idx >= NUM_AEROPORTOS)
        return NULL;

    if (!initialized)
    {
        for (int i = 0; i < NUM_AEROPORTOS; i++)
            utils_aeroporto_codigo(i, codes[i]);
        initialized = 1;
    }

    return codes[idx];
}
