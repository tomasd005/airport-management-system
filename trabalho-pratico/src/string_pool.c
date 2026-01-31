#include "string_pool.h"
#include <glib.h>
#include <pthread.h>

typedef struct {
    GHashTable *map;
    GPtrArray *strings;
    pthread_mutex_t mutex;
} string_pool_t;

static string_pool_t g_pool = {
    .map = NULL,
    .strings = NULL,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

static void string_pool_init_locked(void)
{
    if (!g_pool.map) {
        g_pool.map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        g_pool.strings = g_ptr_array_new();
        g_ptr_array_add(g_pool.strings, NULL);
    }
}

uint32_t string_pool_intern_id(const char *s)
{
    if (!s || !*s)
        return 0;

    pthread_mutex_lock(&g_pool.mutex);
    string_pool_init_locked();

    gpointer encontrado = g_hash_table_lookup(g_pool.map, s);
    if (encontrado) {
        uint32_t id = GPOINTER_TO_UINT(encontrado);
        pthread_mutex_unlock(&g_pool.mutex);
        return id;
    }

    char *dup = g_strdup(s);
    uint32_t id = (uint32_t)g_pool.strings->len;
    g_ptr_array_add(g_pool.strings, dup);
    g_hash_table_insert(g_pool.map, dup, GUINT_TO_POINTER(id));
    pthread_mutex_unlock(&g_pool.mutex);
    return id;
}

const char *string_pool_get(uint32_t id)
{
    if (id == 0 || !g_pool.strings)
        return NULL;
    if (id >= g_pool.strings->len)
        return NULL;
    return (const char *)g_ptr_array_index(g_pool.strings, id);
}

const char *string_pool_intern(const char *s)
{
    uint32_t id = string_pool_intern_id(s);
    if (id == 0)
        return "";
    return string_pool_get(id);
}

void string_pool_clear(void)
{
    pthread_mutex_lock(&g_pool.mutex);
    if (g_pool.map) {
        g_hash_table_destroy(g_pool.map);
        g_pool.map = NULL;
    }
    if (g_pool.strings) {
        g_ptr_array_free(g_pool.strings, TRUE);
        g_pool.strings = NULL;
    }
    pthread_mutex_unlock(&g_pool.mutex);
}
