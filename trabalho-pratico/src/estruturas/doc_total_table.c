#include "estruturas/doc_total_table.h"
#include <stdlib.h>
#include <string.h>

#define LOAD_NUM 7u
#define LOAD_DEN 10u

struct doc_total_table {
    uint32_t *keys;
    double *values;
    size_t size;
    size_t count;
};

static inline uint32_t key_pack(uint32_t key) { return key + 1u; }
static inline uint32_t key_unpack(uint32_t key) { return key - 1u; }

static size_t round_pow2(size_t n)
{
    size_t p = 1;
    while (p < n)
        p <<= 1;
    return p;
}

static inline uint32_t hash_u32(uint32_t k)
{
    return k * 2654435761u;
}

static void table_rehash(doc_total_table_t *tabela, size_t nova_cap)
{
    size_t new_size = round_pow2(nova_cap);
    uint32_t *old_keys = tabela->keys;
    double *old_vals = tabela->values;
    size_t old_size = tabela->size;

    tabela->keys = calloc(new_size, sizeof(uint32_t));
    tabela->values = calloc(new_size, sizeof(double));
    tabela->size = new_size;
    tabela->count = 0;

    for (size_t i = 0; i < old_size; i++) {
        uint32_t packed = old_keys[i];
        if (packed == 0)
            continue;

        uint32_t key = key_unpack(packed);
        double val = old_vals[i];
        size_t mask = new_size - 1;
        size_t idx = hash_u32(packed) & mask;
        while (tabela->keys[idx] != 0)
            idx = (idx + 1) & mask;

        tabela->keys[idx] = packed;
        tabela->values[idx] = val;
        tabela->count++;
    }

    free(old_keys);
    free(old_vals);
}

doc_total_table_t *doc_total_table_create(size_t capacidade_inicial)
{
    doc_total_table_t *t = malloc(sizeof(*t));
    if (!t)
        return NULL;
    size_t cap = round_pow2(capacidade_inicial < 16 ? 16 : capacidade_inicial);
    t->keys = calloc(cap, sizeof(uint32_t));
    t->values = calloc(cap, sizeof(double));
    t->size = cap;
    t->count = 0;
    return t;
}

void doc_total_table_destroy(doc_total_table_t *tabela)
{
    if (!tabela)
        return;
    free(tabela->keys);
    free(tabela->values);
    free(tabela);
}

void doc_total_table_add(doc_total_table_t *tabela, uint32_t key, double delta)
{
    if (!tabela)
        return;

    if ((tabela->count + 1) * LOAD_DEN >= tabela->size * LOAD_NUM)
        table_rehash(tabela, tabela->size * 2);

    uint32_t packed = key_pack(key);
    size_t mask = tabela->size - 1;
    size_t idx = hash_u32(packed) & mask;

    while (1) {
        uint32_t cur = tabela->keys[idx];
        if (cur == 0) {
            tabela->keys[idx] = packed;
            tabela->values[idx] = delta;
            tabela->count++;
            return;
        }
        if (cur == packed) {
            tabela->values[idx] += delta;
            return;
        }
        idx = (idx + 1) & mask;
    }
}

void doc_total_table_foreach(doc_total_table_t *tabela,
                             void (*callback)(uint32_t key, double total, void *user_data),
                             void *user_data)
{
    if (!tabela || !callback)
        return;

    for (size_t i = 0; i < tabela->size; i++) {
        uint32_t packed = tabela->keys[i];
        if (packed == 0)
            continue;
        callback(key_unpack(packed), tabela->values[i], user_data);
    }
}

size_t doc_total_table_size(const doc_total_table_t *tabela)
{
    return tabela ? tabela->count : 0;
}
