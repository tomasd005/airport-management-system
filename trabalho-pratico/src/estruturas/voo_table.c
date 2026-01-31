#include "estruturas/voo_table.h"
#include <stdlib.h>

struct voo_table {
    uint64_t *keys;
    voo_t **values;
    size_t size;
    size_t capacity;
};

static inline uint64_t hash64(uint64_t x)
{
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

static size_t next_pow2(size_t v)
{
    size_t n = 1;
    while (n < v)
        n <<= 1;
    return n;
}

void voo_table_init(voo_table_t *t, size_t capacity)
{
    if (!t)
        return;
    if (capacity < 16)
        capacity = 16;
    capacity = next_pow2(capacity);

    t->keys = calloc(capacity, sizeof(uint64_t));
    t->values = calloc(capacity, sizeof(voo_t *));
    t->capacity = capacity;
    t->size = 0;
}

void voo_table_destroy(voo_table_t *t, void (*destroy)(voo_t *))
{
    if (!t)
        return;
    if (destroy && t->values)
    {
        for (size_t i = 0; i < t->capacity; i++)
        {
            if (t->keys[i])
                destroy(t->values[i]);
        }
    }
    free(t->keys);
    free(t->values);
    t->keys = NULL;
    t->values = NULL;
    t->capacity = 0;
    t->size = 0;
}

voo_table_t *voo_table_create(size_t capacity)
{
    voo_table_t *t = malloc(sizeof(*t));
    if (!t)
        return NULL;
    voo_table_init(t, capacity);
    if (!t->keys || !t->values) {
        voo_table_destroy(t, NULL);
        free(t);
        return NULL;
    }
    return t;
}

void voo_table_free(voo_table_t *t, void (*destroy)(voo_t *))
{
    if (!t)
        return;
    voo_table_destroy(t, destroy);
    free(t);
}

static int voo_table_resize(voo_table_t *t, size_t new_cap)
{
    voo_table_t novo = {0};
    voo_table_init(&novo, new_cap);
    if (!novo.keys || !novo.values)
        return 0;

    for (size_t i = 0; i < t->capacity; i++)
    {
        if (!t->keys[i])
            continue;
        uint64_t key = t->keys[i];
        size_t mask = novo.capacity - 1;
        size_t idx = (size_t)hash64(key) & mask;
        while (novo.keys[idx])
            idx = (idx + 1) & mask;
        novo.keys[idx] = key;
        novo.values[idx] = t->values[i];
        novo.size++;
    }

    free(t->keys);
    free(t->values);
    *t = novo;
    return 1;
}

voo_t *voo_table_lookup(const voo_table_t *t, uint64_t key)
{
    if (!t || !t->keys || key == 0)
        return NULL;

    size_t mask = t->capacity - 1;
    size_t idx = (size_t)hash64(key) & mask;
    while (t->keys[idx])
    {
        if (t->keys[idx] == key)
            return t->values[idx];
        idx = (idx + 1) & mask;
    }
    return NULL;
}

int voo_table_insert(voo_table_t *t, uint64_t key, voo_t *value)
{
    if (!t || !t->keys || key == 0)
        return 0;

    if ((t->size + 1) * 10 >= t->capacity * 7)
    {
        if (!voo_table_resize(t, t->capacity * 2))
            return 0;
    }

    size_t mask = t->capacity - 1;
    size_t idx = (size_t)hash64(key) & mask;
    while (t->keys[idx])
    {
        if (t->keys[idx] == key)
            return 0;
        idx = (idx + 1) & mask;
    }
    t->keys[idx] = key;
    t->values[idx] = value;
    t->size++;
    return 1;
}

void voo_table_foreach(const voo_table_t *t, void (*fn)(voo_t *, void *), void *user_data)
{
    if (!t || !fn || !t->keys)
        return;
    for (size_t i = 0; i < t->capacity; i++)
        if (t->keys[i])
            fn(t->values[i], user_data);
}

size_t voo_table_size(const voo_table_t *t)
{
    return t ? t->size : 0;
}
