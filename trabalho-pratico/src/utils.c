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