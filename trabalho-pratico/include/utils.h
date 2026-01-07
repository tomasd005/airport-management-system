#ifndef UTILS_H
#define UTILS_H

void utils_remove_aspas_somente(char *str);

void utils_remove_aspas(char *str);

void utils_remove_newline(char *str);
char *utils_obtem_nome_ficheiro(const char *caminho);
void utils_trim(char *s);

int utils_parse_date_to_day(const char *date);
int utils_parse_datetime_to_day(const char *datetime);
int utils_parse_datetime_to_minutes(const char *datetime);
int utils_week_from_day(int day);

#endif
