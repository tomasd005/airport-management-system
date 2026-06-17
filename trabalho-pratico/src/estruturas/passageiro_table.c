#include "estruturas/passageiro_table.h"
#include <stdlib.h>

struct passageiro_table
{
    uint32_t *keys;
    passageiro_t **values;
    size_t size;
    size_t capacity;
};

static size_t next_pow2(size_t v)
{
    size_t n = 16;
    while (n < v)
        n <<= 1;
    return n;
}

static inline uint32_t hash32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

static inline uint32_t key_pack(uint32_t key)
{
    return key + 1u;
}

static int passageiro_table_resize(passageiro_table_t *t, size_t new_cap)
{
    passageiro_table_t novo = {0};
    new_cap = next_pow2(new_cap);
    novo.keys = calloc(new_cap, sizeof(uint32_t));
    novo.values = calloc(new_cap, sizeof(passageiro_t *));
    if (!novo.keys || !novo.values)
    {
        free(novo.keys);
        free(novo.values);
        return 0;
    }
    novo.capacity = new_cap;

    for (size_t i = 0; i < t->capacity; i++)
    {
        uint32_t packed = t->keys[i];
        if (!packed)
            continue;

        size_t idx = (size_t)(hash32(packed) & (novo.capacity - 1));
        while (novo.keys[idx])
        {
            idx++;
            idx &= (novo.capacity - 1);
        }
        novo.keys[idx] = packed;
        novo.values[idx] = t->values[i];
        novo.size++;
    }

    free(t->keys);
    free(t->values);
    *t = novo;
    return 1;
}

passageiro_table_t *passageiro_table_create(size_t capacity)
{
    passageiro_table_t *t = malloc(sizeof(*t));
    if (!t)
        return NULL;

    t->keys = NULL;
    t->values = NULL;
    t->size = 0;
    t->capacity = 0;
    if (!passageiro_table_resize(t, capacity))
    {
        free(t);
        return NULL;
    }
    return t;
}

int passageiro_table_reserve(passageiro_table_t *t, size_t capacity)
{
    if (!t)
        return 0;
    capacity = next_pow2(capacity);
    if (capacity <= t->capacity)
        return 1;
    return passageiro_table_resize(t, capacity);
}

void passageiro_table_free(passageiro_table_t *t, void (*destroy)(passageiro_t *))
{
    if (!t)
        return;

    if (destroy)
    {
        for (size_t i = 0; i < t->capacity; i++)
        {
            if (t->keys[i])
                destroy(t->values[i]);
        }
    }

    free(t->keys);
    free(t->values);
    free(t);
}

passageiro_t *passageiro_table_lookup(const passageiro_table_t *t, uint32_t key)
{
    if (!t || !t->keys || key == 0)
        return NULL;

    uint32_t packed = key_pack(key);
    size_t idx = (size_t)(hash32(packed) & (t->capacity - 1));

    while (t->keys[idx])
    {
        if (t->keys[idx] == packed)
            return t->values[idx];
        idx++;
        idx &= (t->capacity - 1);
    }
    return NULL;
}

int passageiro_table_insert(passageiro_table_t *t, uint32_t key, passageiro_t *value)
{
    if (!t || !t->keys || key == 0)
        return 0;

    if ((t->size + 1) * 100 >= t->capacity * 78)
    {
        if (!passageiro_table_resize(t, t->capacity * 2))
            return 0;
    }

    uint32_t packed = key_pack(key);
    size_t idx = (size_t)(hash32(packed) & (t->capacity - 1));

    while (t->keys[idx])
    {
        if (t->keys[idx] == packed)
            return 0;
        idx++;
        idx &= (t->capacity - 1);
    }

    t->keys[idx] = packed;
    t->values[idx] = value;
    t->size++;
    return 1;
}

size_t passageiro_table_size(const passageiro_table_t *t)
{
    return t ? t->size : 0;
}
