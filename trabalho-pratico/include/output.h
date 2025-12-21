#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdio.h>
#include <glib.h>

FILE *create_output_file(int command_number);
void write_line(FILE *f, const char *line);
void close_output_file(FILE *f);

#endif
