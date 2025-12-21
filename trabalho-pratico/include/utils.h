#ifndef UTILS_H
#define UTILS_H

void utils_remove_aspas_somente(char *str);

void utils_remove_aspas(char *str);

void utils_remove_newline(char *str);
char *utils_obtem_nome_ficheiro(const char *caminho);
void utils_trim(char *s);

#endif