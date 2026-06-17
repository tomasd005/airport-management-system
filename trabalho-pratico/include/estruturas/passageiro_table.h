#ifndef PASSAGEIRO_TABLE_H
#define PASSAGEIRO_TABLE_H

#include <stddef.h>
#include <stdint.h>

typedef struct passageiro passageiro_t;

typedef struct passageiro_table passageiro_table_t;

passageiro_table_t *passageiro_table_create(size_t capacity);
int passageiro_table_reserve(passageiro_table_t *t, size_t capacity);
void passageiro_table_free(passageiro_table_t *t, void (*destroy)(passageiro_t *));
passageiro_t *passageiro_table_lookup(const passageiro_table_t *t, uint32_t key);
int passageiro_table_insert(passageiro_table_t *t, uint32_t key, passageiro_t *value);
size_t passageiro_table_size(const passageiro_table_t *t);

#endif
