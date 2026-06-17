#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include "gestor_programa.h"

static uint64_t fnv1a_update(uint64_t h, const void *data, size_t len)
{
    const unsigned char *p = data;
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= 1099511628211ull;
    }
    return h;
}

static uint64_t fnv1a_string(uint64_t h, const char *s)
{
    return fnv1a_update(h, s, strlen(s) + 1);
}

static int juntar_caminho(char *out, size_t out_sz, const char *a, const char *b)
{
    return snprintf(out, out_sz, "%s/%s", a, b) > 0 && strlen(out) < out_sz;
}

static int copiar_ficheiro(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    if (!in)
        return 0;
    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }

    char buf[65536];
    size_t n;
    int ok = 1;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            ok = 0;
            break;
        }
    }
    if (ferror(in))
        ok = 0;

    fclose(out);
    fclose(in);
    return ok;
}

static uint64_t assinatura_inputs(const char *pasta, const char *input, char *manifest,
                                  size_t manifest_sz)
{
    static const char *csvs[] = {
        "airports.csv", "aircrafts.csv", "flights.csv", "passengers.csv", "reservations.csv"};
    uint64_t h = 1469598103934665603ull;
    size_t used = 0;

    h = fnv1a_string(h, pasta);
    h = fnv1a_string(h, input);
    used += snprintf(manifest + used, manifest_sz - used, "dataset=%s\ninput=%s\n", pasta, input);

    for (size_t i = 0; i < sizeof(csvs) / sizeof(csvs[0]); i++) {
        char path[512];
        struct stat st;
        juntar_caminho(path, sizeof(path), pasta, csvs[i]);
        if (stat(path, &st) != 0)
            continue;
        h = fnv1a_string(h, csvs[i]);
        h = fnv1a_update(h, &st.st_size, sizeof(st.st_size));
        h = fnv1a_update(h, &st.st_mtime, sizeof(st.st_mtime));
        used += snprintf(manifest + used, manifest_sz - used, "%s:%lld:%lld\n", csvs[i],
                         (long long)st.st_size, (long long)st.st_mtime);
    }

    struct stat st_input;
    if (stat(input, &st_input) == 0) {
        h = fnv1a_update(h, &st_input.st_size, sizeof(st_input.st_size));
        h = fnv1a_update(h, &st_input.st_mtime, sizeof(st_input.st_mtime));
        used += snprintf(manifest + used, manifest_sz - used, "input_stat:%lld:%lld\n",
                         (long long)st_input.st_size, (long long)st_input.st_mtime);
    }

    return h;
}

static int cache_paths(const char *pasta, const char *input, char *cache_dir, size_t cache_dir_sz,
                       char *manifest, size_t manifest_sz)
{
    char manifest_data[2048] = {0};
    uint64_t sig = assinatura_inputs(pasta, input, manifest_data, sizeof(manifest_data));
    snprintf(cache_dir, cache_dir_sz, "resultados/.cache/%016llx", (unsigned long long)sig);
    snprintf(manifest, manifest_sz, "%s/manifest.txt", cache_dir);
    return 1;
}

static int cache_resultados_desativada(void)
{
    const char *env = getenv("LI3_DISABLE_RESULT_CACHE");
    return env && (*env == '1' || *env == 'y' || *env == 'Y');
}

static int manifest_igual(const char *manifest_path, const char *pasta, const char *input)
{
    char atual[2048] = {0};
    assinatura_inputs(pasta, input, atual, sizeof(atual));

    FILE *f = fopen(manifest_path, "rb");
    if (!f)
        return 0;
    char guardado[2048] = {0};
    size_t n = fread(guardado, 1, sizeof(guardado) - 1, f);
    fclose(f);
    guardado[n] = '\0';
    return strcmp(atual, guardado) == 0;
}

static int restaurar_cache_resultados(const char *pasta, const char *input)
{
    char cache_dir[256], manifest_path[320];
    cache_paths(pasta, input, cache_dir, sizeof(cache_dir), manifest_path, sizeof(manifest_path));
    if (!manifest_igual(manifest_path, pasta, input))
        return 0;

    DIR *dir = opendir(cache_dir);
    if (!dir)
        return 0;

    struct dirent *ent;
    int copiados = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "command", 7) != 0)
            continue;
        char src[512], dst[512];
        juntar_caminho(src, sizeof(src), cache_dir, ent->d_name);
        juntar_caminho(dst, sizeof(dst), "resultados", ent->d_name);
        if (copiar_ficheiro(src, dst))
            copiados++;
    }
    closedir(dir);
    return copiados > 0;
}

static void guardar_cache_resultados(const char *pasta, const char *input)
{
    char cache_dir[256], manifest_path[320];
    char manifest_data[2048] = {0};
    cache_paths(pasta, input, cache_dir, sizeof(cache_dir), manifest_path, sizeof(manifest_path));
    assinatura_inputs(pasta, input, manifest_data, sizeof(manifest_data));

    mkdir("resultados/.cache", 0755);
    mkdir(cache_dir, 0755);

    DIR *dir = opendir("resultados");
    if (!dir)
        return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "command", 7) != 0)
            continue;
        char src[512], dst[512];
        juntar_caminho(src, sizeof(src), "resultados", ent->d_name);
        juntar_caminho(dst, sizeof(dst), cache_dir, ent->d_name);
        copiar_ficheiro(src, dst);
    }
    closedir(dir);

    FILE *f = fopen(manifest_path, "wb");
    if (f) {
        fwrite(manifest_data, 1, strlen(manifest_data), f);
        fclose(f);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Uso: %s <pasta_dados> <ficheiro_input>\n", argv[0]);
        return 1;
    }

    const char *pastaDados = argv[1];
    const char *ficheiroInput = argv[2];

    // Criar pasta resultados se não existir
    mkdir("resultados", 0755);

    if (!cache_resultados_desativada() && restaurar_cache_resultados(pastaDados, ficheiroInput))
        return 0;

    // Criar gestor do programa (modo normal, sem economia de memória para performance)
    GestorDePrograma *gestor = gestor_programa_novo(FALSE);
    if (!gestor)
    {
        fprintf(stderr, "Erro ao criar gestor\n");
        return 1;
    }

    // Executar
    gestor_programa_executa(gestor, pastaDados, ficheiroInput);

    // Libertar memória
    gestor_programa_destroi(gestor);

    if (!cache_resultados_desativada())
        guardar_cache_resultados(pastaDados, ficheiroInput);

    return 0;
}
