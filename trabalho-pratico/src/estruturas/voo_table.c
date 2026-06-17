#include "estruturas/voo_table.h"
#include <stdlib.h>

struct voo_table {
    uint64_t *keys;
    uint32_t *values;
    uint32_t *ids;
    size_t ids_capacity;
    size_t size;
    size_t capacity;
};

static inline uint64_t hash64(uint64_t x)
{
    x *= 11400714819323198485ull;
    return x ^ (x >> 32);
}

static size_t round_pow2(size_t v)
{
    size_t n = 16;
    while (n < v)
        n <<= 1;
    return n;
}

static int voo_table_resize(voo_table_t *t, size_t new_cap);

void voo_table_init(voo_table_t *t, size_t capacity)
{
    if (!t)
        return;
    capacity = round_pow2(capacity);

    t->keys = calloc(capacity, sizeof(uint64_t));
    t->values = calloc(capacity, sizeof(uint32_t));
    t->ids = NULL;
    t->ids_capacity = 0;
    t->capacity = capacity;
    t->size = 0;
}

void voo_table_destroy(voo_table_t *t)
{
    if (!t)
        return;
    free(t->keys);
    free(t->values);
    free(t->ids);
    t->keys = NULL;
    t->values = NULL;
    t->ids = NULL;
    t->ids_capacity = 0;
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
        voo_table_destroy(t);
        free(t);
        return NULL;
    }
    return t;
}

int voo_table_reserve(voo_table_t *t, size_t capacity)
{
    if (!t)
        return 0;
    capacity = round_pow2(capacity);
    if (capacity <= t->capacity)
        return 1;
    if (!voo_table_resize(t, capacity))
        return 0;

    if (t->ids_capacity < capacity)
    {
        uint32_t *new_ids = realloc(t->ids, capacity * sizeof(uint32_t));
        if (!new_ids)
            return 0;
        t->ids = new_ids;
        t->ids_capacity = capacity;
    }
    return 1;
}

void voo_table_free(voo_table_t *t)
{
    if (!t)
        return;
    voo_table_destroy(t);
    free(t);
}

static int voo_table_resize(voo_table_t *t, size_t new_cap)
{
    voo_table_t novo = {0};
    voo_table_init(&novo, round_pow2(new_cap));
    if (!novo.keys || !novo.values)
        return 0;

    novo.ids = t->ids;
    novo.ids_capacity = t->ids_capacity;

    for (size_t i = 0; i < t->capacity; i++)
    {
        if (!t->keys[i])
            continue;
        uint64_t key = t->keys[i];
        size_t idx = (size_t)(hash64(key) & (novo.capacity - 1));
        while (novo.keys[idx])
        {
            idx++;
            idx &= (novo.capacity - 1);
        }
        novo.keys[idx] = key;
        novo.values[idx] = t->values[i];
        novo.size++;
    }

    free(t->keys);
    free(t->values);
    *t = novo;
    return 1;
}

int voo_table_lookup_id(const voo_table_t *t, uint64_t key, uint32_t *out_id)
{
    if (!t || !t->keys || key == 0)
        return 0;

    size_t idx = (size_t)(hash64(key) & (t->capacity - 1));
    while (t->keys[idx])
    {
        if (t->keys[idx] == key)
        {
            if (out_id)
                *out_id = t->values[idx];
            return 1;
        }
        idx++;
        idx &= (t->capacity - 1);
    }
    return 0;
}

int voo_table_insert_id(voo_table_t *t, uint64_t key, uint32_t id)
{
    if (!t || !t->keys || key == 0)
        return 0;

    if ((t->size + 1) * 100 >= t->capacity * 78)
    {
        if (!voo_table_resize(t, t->capacity * 2))
            return 0;
    }

    size_t idx = (size_t)(hash64(key) & (t->capacity - 1));
    while (t->keys[idx])
    {
        if (t->keys[idx] == key)
            return 0;
        idx++;
        idx &= (t->capacity - 1);
    }

    if (t->size == t->ids_capacity)
    {
        size_t new_cap = t->ids_capacity ? t->ids_capacity * 2 : 1024;
        uint32_t *new_ids = realloc(t->ids, new_cap * sizeof(uint32_t));
        if (!new_ids)
            return 0;
        t->ids = new_ids;
        t->ids_capacity = new_cap;
    }

    t->keys[idx] = key;
    t->values[idx] = id;
    t->ids[t->size] = id;
    t->size++;
    return 1;
}

void voo_table_foreach_id(const voo_table_t *t, void (*fn)(uint32_t id, void *), void *user_data)
{
    if (!t || !fn || !t->keys)
        return;
    for (size_t i = 0; i < t->size; i++)
        fn(t->ids[i], user_data);
}

size_t voo_table_size(const voo_table_t *t)
{
    return t ? t->size : 0;
}
