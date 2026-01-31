#include "gestor_interativo.h"
#include "gestores/gestor_aeroportos.h"
#include "gestores/gestor_avioes.h"
#include "gestores/gestor_voos.h"
#include "gestores/gestor_passageiros.h"
#include "gestores/gestor_reservas.h"
#include "queries/querie1.h"
#include "queries/querie2.h"
#include "queries/querie3.h"
#include "queries/querie4.h"
#include "queries/querie5.h"
#include "queries/querie6.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define BUFFER_SIZE 512
#define DEFAULT_DATASET "./dataset"
#define HISTORY_MAX 50
#define HISTORY_LEN 128

/* Códigos ANSI para cores no terminal */
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

typedef struct {
    int id;
    int n;
    char p1[128];
    char p2[128];
    int has_period;
    double last_ms;
    char comando[128];
} last_query_t;

/**
 * @struct GestorInterativo
 * @brief Estrutura do gestor do modo interativo.
 *
 * Contém os gestores de dados necessários à execução das queries
 * e um indicador que sinaliza se os dados já foram carregados.
 */
struct GestorInterativo {
    gestor_aeroportos_t *aeroportos;
    gestor_avioes_t *avioes;
    gestor_voos_t *voos;
    gestor_passageiros_t *passageiros;
    gestor_reservas_t *reservas;
    int dados_carregados;
    char dataset_path[BUFFER_SIZE];
    last_query_t last_query;
    int has_last_query;
    char history[HISTORY_MAX][HISTORY_LEN];
    int history_count;
    int history_pos;
};

/**
 * @brief Cria um novo gestor para o modo interativo.
 *
 * Inicializa todos os gestores de dados necessários.
 *
 * @return Ponteiro para o gestor interativo criado ou NULL em erro
 */
gestor_interativo_t *gestor_interativo_criar(void)
{
    gestor_interativo_t *gestor = malloc(sizeof(gestor_interativo_t));
    if (!gestor)
        return NULL;

    gestor->aeroportos = gestor_aeroportos_criar();
    gestor->avioes = gestor_avioes_criar();
    gestor->voos = gestor_voos_criar();
    gestor->passageiros = gestor_passageiros_criar();
    gestor->reservas = gestor_reservas_criar();
    gestor->dados_carregados = 0;
    gestor->dataset_path[0] = '\0';
    gestor->has_last_query = 0;
    gestor->history_count = 0;
    gestor->history_pos = 0;

    return gestor;
}

/**
 * @brief Destrói o gestor interativo e liberta a memória associada.
 *
 * @param gestor Ponteiro para o gestor interativo
 */
void gestor_interativo_destruir(gestor_interativo_t *gestor)
{
    if (!gestor)
        return;

    gestor_aeroportos_destruir(gestor->aeroportos);
    gestor_avioes_destruir(gestor->avioes);
    gestor_voos_destruir(gestor->voos);
    gestor_passageiros_destruir(gestor->passageiros);
    gestor_reservas_destruir(gestor->reservas);

    free(gestor);
}

/**
 * @brief Remove espaços e quebras de linha de uma string.
 *
 * @param str String a limpar
 */
static void limpar_input(char *str)
{
    if (!str)
        return;

    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }

    char *end = str + strlen(str) - 1;
    while (end >= str && isspace(*end)) {
        *end = '\0';
        end--;
    }

    char *start = str;
    while (*start && isspace(*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);
}

static void limpar_ecra(void)
{
    printf("\033[2J\033[H");
}

static double tempo_ms(struct timespec inicio, struct timespec fim)
{
    double sec = (double)(fim.tv_sec - inicio.tv_sec) * 1000.0;
    double nsec = (double)(fim.tv_nsec - inicio.tv_nsec) / 1000000.0;
    return sec + nsec;
}

static void history_add(gestor_interativo_t *gestor, const char *cmd)
{
    if (!gestor || !cmd || !*cmd)
        return;

    int idx = gestor->history_pos % HISTORY_MAX;
    snprintf(gestor->history[idx], HISTORY_LEN, "%s", cmd);
    gestor->history_pos++;
    if (gestor->history_count < HISTORY_MAX)
        gestor->history_count++;
}

static void history_show(gestor_interativo_t *gestor)
{
    if (!gestor || gestor->history_count == 0) {
        printf(YELLOW "Sem histórico ainda.\n" RESET);
        return;
    }

    printf(BOLD "\nHistórico de comandos\n" RESET);
    int start = gestor->history_pos - gestor->history_count;
    for (int i = 0; i < gestor->history_count; i++) {
        int idx = (start + i) % HISTORY_MAX;
        if (idx < 0)
            idx += HISTORY_MAX;
        printf("  %2d) %s\n", i + 1, gestor->history[idx]);
    }
}

static void copiar_dataset_padrao(char *dest, size_t dest_size)
{
    if (!dest || dest_size == 0)
        return;
    snprintf(dest, dest_size, "%s", DEFAULT_DATASET);
}

static int ler_linha(const char *prompt, char *buf, size_t size, int obrigatorio)
{
    if (!buf || size == 0)
        return 0;

    if (prompt)
        printf("%s", prompt);

    if (!fgets(buf, size, stdin))
        return 0;

    limpar_input(buf);
    if (obrigatorio && buf[0] == '\0')
        return 0;

    return 1;
}

static FILE *pedir_saida_ficheiro(char *caminho, size_t tamanho)
{
    char resposta[8];

    if (!ler_linha("Gravar resultado em ficheiro? (s/n): ", resposta, sizeof(resposta), 0))
        return stdout;

    if (resposta[0] != 's' && resposta[0] != 'S')
        return stdout;

    if (!ler_linha("Caminho de saída: ", caminho, tamanho, 1)) {
        printf(RED "✗ Caminho inválido.\n" RESET);
        return stdout;
    }

    FILE *out = fopen(caminho, "w");
    if (!out) {
        printf(RED "✗ Não foi possível abrir '%s' (%s)\n" RESET, caminho, strerror(errno));
        return stdout;
    }

    return out;
}

static void fechar_saida(FILE *out, const char *caminho)
{
    if (out && out != stdout) {
        fclose(out);
        printf(GREEN "✓ Resultado guardado em: %s\n" RESET, caminho);
    }
}

static void mostrar_estatisticas(gestor_interativo_t *gestor)
{
    if (!gestor || !gestor->dados_carregados) {
        printf(RED "✗ Dataset não carregado.\n" RESET);
        return;
    }

    printf(BOLD "\nEstatísticas do Dataset\n" RESET);
    printf("  Caminho: %s\n", gestor->dataset_path[0] ? gestor->dataset_path : "(desconhecido)");
    printf("  Aeroportos:   %u\n", gestor_aeroportos_contar(gestor->aeroportos));
    printf("  Aviões:       %u\n", gestor_avioes_contar(gestor->avioes));
    printf("  Voos:         %u\n", gestor_voos_contar(gestor->voos));
    printf("  Passageiros:  %u\n", gestor_passageiros_numero(gestor->passageiros));
    printf("  Reservas:     %u\n", gestor_reservas_numero(gestor->reservas));
}

static void mostrar_ultima_query(gestor_interativo_t *gestor)
{
    if (!gestor || !gestor->has_last_query) {
        printf(YELLOW "Sem última query registada.\n" RESET);
        return;
    }

    printf(BOLD "\nÚltima query executada\n" RESET);
    printf("  Comando: %s\n", gestor->last_query.comando[0] ? gestor->last_query.comando : "(desconhecido)");
    printf("  Tempo:   %.2f ms\n", gestor->last_query.last_ms);
}

static int reiniciar_gestores(gestor_interativo_t *gestor)
{
    if (!gestor)
        return 0;

    gestor_aeroportos_destruir(gestor->aeroportos);
    gestor_avioes_destruir(gestor->avioes);
    gestor_voos_destruir(gestor->voos);
    gestor_passageiros_destruir(gestor->passageiros);
    gestor_reservas_destruir(gestor->reservas);

    gestor->aeroportos = gestor_aeroportos_criar();
    gestor->avioes = gestor_avioes_criar();
    gestor->voos = gestor_voos_criar();
    gestor->passageiros = gestor_passageiros_criar();
    gestor->reservas = gestor_reservas_criar();
    gestor->dados_carregados = 0;

    if (!gestor->aeroportos || !gestor->avioes || !gestor->voos || !gestor->passageiros ||
        !gestor->reservas)
        return 0;

    return 1;
}

/**
 * @brief Carrega todos os ficheiros CSV do dataset.
 *
 * @param gestor Gestor interativo
 * @param pasta Caminho para a pasta do dataset
 */
static void carregar_dataset(gestor_interativo_t *gestor, const char *pasta)
{
    printf(CYAN "Carregando dataset de: %s\n" RESET, pasta);
    printf("Isso pode demorar alguns segundos...\n\n");

    char caminho[BUFFER_SIZE];

    printf("  [1/5] Carregando aeroportos...\n");
    snprintf(caminho, sizeof(caminho), "%s/airports.csv", pasta);
    gestor_aeroportos_carregar(gestor->aeroportos, caminho);

    printf("  [2/5] Carregando aviões...\n");
    snprintf(caminho, sizeof(caminho), "%s/aircrafts.csv", pasta);
    gestor_avioes_carregar(gestor->avioes, caminho);

    printf("  [3/5] Carregando voos...\n");
    snprintf(caminho, sizeof(caminho), "%s/flights.csv", pasta);
    gestor_voos_carregar_com_validacao(gestor->voos, caminho, gestor->avioes);

    printf("  [4/5] Carregando passageiros...\n");
    snprintf(caminho, sizeof(caminho), "%s/passengers.csv", pasta);
    gestor_passageiros_carregar(gestor->passageiros, caminho);

    printf("  [5/5] Carregando reservas...\n");
    snprintf(caminho, sizeof(caminho), "%s/reservations.csv", pasta);
    gestor_reservas_carregar_com_validacao(gestor->reservas, caminho, gestor->voos,
                                           gestor->passageiros);

    gestor_reservas_finalizar(gestor->reservas);
    {
        GHashTable *docs_top10 = g_hash_table_new(g_direct_hash, g_direct_equal);
        gestor_reservas_coletar_docs_top10(gestor->reservas, docs_top10);
        gestor_passageiros_carregar_detalhes(gestor->passageiros, docs_top10);
        g_hash_table_destroy(docs_top10);
    }
    gestor_voos_atualizar_contagens_aeroportos(gestor->voos, gestor->aeroportos);
    gestor_voos_preparar_q3(gestor->voos);

    gestor->dados_carregados = 1;
    snprintf(gestor->dataset_path, sizeof(gestor->dataset_path), "%s", pasta);

    printf(GREEN "\n✓ Dataset carregado com sucesso!\n" RESET);
    printf("   Aeroportos: %u\n", gestor_aeroportos_contar(gestor->aeroportos));
    printf("   Aviões: %u\n", gestor_avioes_contar(gestor->avioes));
    printf("   Voos: %u\n", gestor_voos_contar(gestor->voos));
    printf("   Passageiros: %u\n", gestor_passageiros_numero(gestor->passageiros));
    printf("   Reservas: %u\n\n", gestor_reservas_numero(gestor->reservas));
}

/**
 * @brief Apresenta o menu principal do modo interativo.
 */
static void mostrar_menu(void)
{
    printf(BOLD BLUE "\n╔═══════════════════════════════════════════════╗\n" RESET);
    printf(BOLD BLUE "║" RESET "      SISTEMA DE GESTÃO DE VOOS - LI3      " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "╠═══════════════════════════════════════════════╣\n" RESET);
    printf(BOLD BLUE "║" RESET " " BOLD "Queries Disponíveis:" RESET
                     "                        " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET "                                            " BOLD BLUE
                     "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "1" RESET
                     " - Resumo de aeroporto                    " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "2" RESET
                     " - Top N aviões com mais voos             " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "3" RESET
                     " - Aeroporto com mais partidas (período)  " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "4" RESET
                     " - Passageiro no top 10 mais vezes        " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "5" RESET
                     " - Companhias com mais atrasos            " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " GREEN "6" RESET
                     " - Destino mais comum (nacionalidade)     " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET "                                            " BOLD BLUE
                     "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " MAGENTA "7" RESET
                     " - Estatísticas do dataset                " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " MAGENTA "8" RESET
                     " - Recarregar dataset                      " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " MAGENTA "9" RESET
                     " - Ajuda/Atalhos                           " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " MAGENTA "10" RESET
                     " - Histórico de comandos                   " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " MAGENTA "11" RESET
                     " - Última query (detalhes)                 " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " MAGENTA "12" RESET
                     " - Repetir última query                     " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "║" RESET "                                            " BOLD BLUE
                     "║\n" RESET);
    printf(BOLD BLUE "║" RESET " " YELLOW "0" RESET
                     " - Sair                                   " BOLD BLUE "║\n" RESET);
    printf(BOLD BLUE "╚═══════════════════════════════════════════════╝\n" RESET);
    printf("Atalhos: " CYAN "q1..q6" RESET ", " CYAN "stats" RESET ", " CYAN "reload" RESET
           ", " CYAN "clear" RESET ", " CYAN "history" RESET ", " CYAN "repeat" RESET
           ", " CYAN "last" RESET "\n");
}

static void executar_query1(gestor_interativo_t *gestor)
{
    char codigo[10];
    char caminho_saida[BUFFER_SIZE];

    if (!gestor || !gestor->dados_carregados) {
        printf(RED "✗ Dataset não carregado.\n" RESET);
        return;
    }

    printf(CYAN "\n→ Query 1: Resumo de Aeroporto\n" RESET);
    printf("Código do aeroporto (ex: OPO): ");

    if (!fgets(codigo, sizeof(codigo), stdin))
        return;
    limpar_input(codigo);

    if (strlen(codigo) == 0) {
        printf(RED "✗ Código inválido!\n" RESET);
        return;
    }

    FILE *out = pedir_saida_ficheiro(caminho_saida, sizeof(caminho_saida));
    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[32];
    snprintf(comando, sizeof(comando), "1 %s", codigo);
    gestor->last_query.id = 1;
    gestor->last_query.n = 0;
    snprintf(gestor->last_query.p1, sizeof(gestor->last_query.p1), "%s", codigo);
    gestor->last_query.p2[0] = '\0';
    gestor->last_query.has_period = 0;
    gestor->has_last_query = 1;
    history_add(gestor, comando);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    query1(gestor->aeroportos, gestor->voos, gestor->reservas, comando, codigo, out);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    gestor->last_query.last_ms = tempo_ms(t0, t1);
    snprintf(gestor->last_query.comando, sizeof(gestor->last_query.comando), "%s", comando);
    fechar_saida(out, caminho_saida);
    printf(CYAN "Tempo de execução: %.2f ms\n" RESET, gestor->last_query.last_ms);
}

static void executar_query2(gestor_interativo_t *gestor)
{
    char input_n[20], fabricante[100];
    char caminho_saida[BUFFER_SIZE];

    if (!gestor || !gestor->dados_carregados) {
        printf(RED "✗ Dataset não carregado.\n" RESET);
        return;
    }

    printf(CYAN "\n→ Query 2: Top N Aviões com Mais Voos\n" RESET);
    printf("Número de aviões (N): ");

    if (!fgets(input_n, sizeof(input_n), stdin))
        return;
    limpar_input(input_n);

    int N = atoi(input_n);
    if (N <= 0) {
        printf(RED "✗ Número inválido!\n" RESET);
        return;
    }

    printf("Fabricante (opcional, Enter para todos): ");
    if (!fgets(fabricante, sizeof(fabricante), stdin))
        return;
    limpar_input(fabricante);

    char *fab = (strlen(fabricante) > 0) ? fabricante : NULL;

    FILE *out = pedir_saida_ficheiro(caminho_saida, sizeof(caminho_saida));
    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[128];
    snprintf(comando, sizeof(comando), "2 %d %s", N, fab ? fab : "");
    gestor->last_query.id = 2;
    gestor->last_query.n = N;
    snprintf(gestor->last_query.p1, sizeof(gestor->last_query.p1), "%s", fab ? fab : "");
    gestor->last_query.p2[0] = '\0';
    gestor->last_query.has_period = 0;
    gestor->has_last_query = 1;
    history_add(gestor, comando);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    query2(gestor->avioes, gestor->voos, N, fab, comando, out);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    gestor->last_query.last_ms = tempo_ms(t0, t1);
    snprintf(gestor->last_query.comando, sizeof(gestor->last_query.comando), "%s", comando);
    fechar_saida(out, caminho_saida);
    printf(CYAN "Tempo de execução: %.2f ms\n" RESET, gestor->last_query.last_ms);
}

static void executar_query3(gestor_interativo_t *gestor)
{
    char data_inicio[20], data_fim[20];
    char caminho_saida[BUFFER_SIZE];

    if (!gestor || !gestor->dados_carregados) {
        printf(RED "✗ Dataset não carregado.\n" RESET);
        return;
    }

    printf(CYAN "\n→ Query 3: Aeroporto com Mais Partidas (Período)\n" RESET);
    printf("Data inicial (YYYY-MM-DD): ");

    if (!fgets(data_inicio, sizeof(data_inicio), stdin))
        return;
    limpar_input(data_inicio);

    printf("Data final (YYYY-MM-DD): ");
    if (!fgets(data_fim, sizeof(data_fim), stdin))
        return;
    limpar_input(data_fim);

    if (strlen(data_inicio) < 10 || strlen(data_fim) < 10) {
        printf(RED "✗ Datas inválidas!\n" RESET);
        return;
    }

    FILE *out = pedir_saida_ficheiro(caminho_saida, sizeof(caminho_saida));
    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[64];
    snprintf(comando, sizeof(comando), "3 %s %s", data_inicio, data_fim);
    gestor->last_query.id = 3;
    gestor->last_query.n = 0;
    snprintf(gestor->last_query.p1, sizeof(gestor->last_query.p1), "%s", data_inicio);
    snprintf(gestor->last_query.p2, sizeof(gestor->last_query.p2), "%s", data_fim);
    gestor->last_query.has_period = 1;
    gestor->has_last_query = 1;
    history_add(gestor, comando);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    query3(gestor->aeroportos, gestor->voos, data_inicio, data_fim, comando, out);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    gestor->last_query.last_ms = tempo_ms(t0, t1);
    snprintf(gestor->last_query.comando, sizeof(gestor->last_query.comando), "%s", comando);
    fechar_saida(out, caminho_saida);
    printf(CYAN "Tempo de execução: %.2f ms\n" RESET, gestor->last_query.last_ms);
}

static void executar_query4(gestor_interativo_t *gestor)
{
    char resposta[10], data_inicio[20], data_fim[20];
    char caminho_saida[BUFFER_SIZE];

    if (!gestor || !gestor->dados_carregados) {
        printf(RED "✗ Dataset não carregado.\n" RESET);
        return;
    }

    printf(CYAN "\n→ Query 4: Passageiro no Top 10 Mais Vezes\n" RESET);
    printf("Filtrar por período? (s/n): ");

    if (!fgets(resposta, sizeof(resposta), stdin))
        return;
    limpar_input(resposta);

    char *di = NULL, *df = NULL;

    if (resposta[0] == 's' || resposta[0] == 'S') {
        printf("Data inicial (YYYY-MM-DD): ");
        if (!fgets(data_inicio, sizeof(data_inicio), stdin))
            return;
        limpar_input(data_inicio);

        printf("Data final (YYYY-MM-DD): ");
        if (!fgets(data_fim, sizeof(data_fim), stdin))
            return;
        limpar_input(data_fim);

        di = data_inicio;
        df = data_fim;
    }

    FILE *out = pedir_saida_ficheiro(caminho_saida, sizeof(caminho_saida));
    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[64];
    if (di && df)
        snprintf(comando, sizeof(comando), "4 %s %s", di, df);
    else
        snprintf(comando, sizeof(comando), "4");

    gestor->last_query.id = 4;
    gestor->last_query.n = 0;
    snprintf(gestor->last_query.p1, sizeof(gestor->last_query.p1), "%s", di ? di : "");
    snprintf(gestor->last_query.p2, sizeof(gestor->last_query.p2), "%s", df ? df : "");
    gestor->last_query.has_period = (di && df) ? 1 : 0;
    gestor->has_last_query = 1;
    history_add(gestor, comando);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    query4(gestor->reservas, gestor->voos, gestor->passageiros, di, df, comando, out);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    gestor->last_query.last_ms = tempo_ms(t0, t1);
    snprintf(gestor->last_query.comando, sizeof(gestor->last_query.comando), "%s", comando);
    fechar_saida(out, caminho_saida);
    printf(CYAN "Tempo de execução: %.2f ms\n" RESET, gestor->last_query.last_ms);
}

static void executar_query5(gestor_interativo_t *gestor)
{
    char input_n[20];
    char caminho_saida[BUFFER_SIZE];

    if (!gestor || !gestor->dados_carregados) {
        printf(RED "✗ Dataset não carregado.\n" RESET);
        return;
    }

    printf(CYAN "\n→ Query 5: Companhias com Mais Atrasos\n" RESET);
    printf("Número de companhias (N): ");

    if (!fgets(input_n, sizeof(input_n), stdin))
        return;
    limpar_input(input_n);

    int N = atoi(input_n);
    if (N <= 0) {
        printf(RED "✗ Número inválido!\n" RESET);
        return;
    }

    FILE *out = pedir_saida_ficheiro(caminho_saida, sizeof(caminho_saida));
    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[32];
    snprintf(comando, sizeof(comando), "5 %d", N);
    gestor->last_query.id = 5;
    gestor->last_query.n = N;
    gestor->last_query.p1[0] = '\0';
    gestor->last_query.p2[0] = '\0';
    gestor->last_query.has_period = 0;
    gestor->has_last_query = 1;
    history_add(gestor, comando);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    query5(gestor->voos, N, comando, out);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    gestor->last_query.last_ms = tempo_ms(t0, t1);
    snprintf(gestor->last_query.comando, sizeof(gestor->last_query.comando), "%s", comando);
    fechar_saida(out, caminho_saida);
    printf(CYAN "Tempo de execução: %.2f ms\n" RESET, gestor->last_query.last_ms);
}

static void executar_query6(gestor_interativo_t *gestor)
{
    char nacionalidade[100];
    char caminho_saida[BUFFER_SIZE];

    if (!gestor || !gestor->dados_carregados) {
        printf(RED "✗ Dataset não carregado.\n" RESET);
        return;
    }

    printf(CYAN "\n→ Query 6: Destino Mais Comum (Nacionalidade)\n" RESET);
    printf("Nacionalidade: ");

    if (!fgets(nacionalidade, sizeof(nacionalidade), stdin))
        return;
    limpar_input(nacionalidade);

    if (strlen(nacionalidade) == 0) {
        printf(RED "✗ Nacionalidade inválida!\n" RESET);
        return;
    }

    FILE *out = pedir_saida_ficheiro(caminho_saida, sizeof(caminho_saida));
    printf("\n" BOLD "Resultado:\n" RESET);
    char comando[128];
    snprintf(comando, sizeof(comando), "6 %s", nacionalidade);
    gestor->last_query.id = 6;
    gestor->last_query.n = 0;
    snprintf(gestor->last_query.p1, sizeof(gestor->last_query.p1), "%s", nacionalidade);
    gestor->last_query.p2[0] = '\0';
    gestor->last_query.has_period = 0;
    gestor->has_last_query = 1;
    history_add(gestor, comando);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    query6(gestor->reservas, gestor->voos, gestor->passageiros, nacionalidade, comando, out);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    gestor->last_query.last_ms = tempo_ms(t0, t1);
    snprintf(gestor->last_query.comando, sizeof(gestor->last_query.comando), "%s", comando);
    fechar_saida(out, caminho_saida);
    printf(CYAN "Tempo de execução: %.2f ms\n" RESET, gestor->last_query.last_ms);
}

static void executar_ultima_query(gestor_interativo_t *gestor)
{
    char caminho_saida[BUFFER_SIZE];

    if (!gestor || !gestor->has_last_query) {
        printf(YELLOW "Sem query anterior para repetir.\n" RESET);
        return;
    }
    if (!gestor->dados_carregados) {
        printf(RED "✗ Dataset não carregado.\n" RESET);
        return;
    }

    FILE *out = pedir_saida_ficheiro(caminho_saida, sizeof(caminho_saida));
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    switch (gestor->last_query.id) {
    case 1: {
        char comando[64];
        snprintf(comando, sizeof(comando), "1 %s", gestor->last_query.p1);
        query1(gestor->aeroportos, gestor->voos, gestor->reservas, comando, gestor->last_query.p1,
               out);
        break;
    }
    case 2: {
        char comando[128];
        const char *fab = gestor->last_query.p1[0] ? gestor->last_query.p1 : NULL;
        snprintf(comando, sizeof(comando), "2 %d %s", gestor->last_query.n, fab ? fab : "");
        query2(gestor->avioes, gestor->voos, gestor->last_query.n, fab, comando, out);
        break;
    }
    case 3: {
        char comando[128];
        snprintf(comando, sizeof(comando), "3 %s %s", gestor->last_query.p1, gestor->last_query.p2);
        query3(gestor->aeroportos, gestor->voos, gestor->last_query.p1, gestor->last_query.p2,
               comando, out);
        break;
    }
    case 4: {
        char comando[128];
        const char *di = gestor->last_query.has_period ? gestor->last_query.p1 : NULL;
        const char *df = gestor->last_query.has_period ? gestor->last_query.p2 : NULL;
        if (di && df)
            snprintf(comando, sizeof(comando), "4 %s %s", di, df);
        else
            snprintf(comando, sizeof(comando), "4");
        query4(gestor->reservas, gestor->voos, gestor->passageiros, di, df, comando, out);
        break;
    }
    case 5: {
        char comando[64];
        snprintf(comando, sizeof(comando), "5 %d", gestor->last_query.n);
        query5(gestor->voos, gestor->last_query.n, comando, out);
        break;
    }
    case 6: {
        char comando[128];
        snprintf(comando, sizeof(comando), "6 %s", gestor->last_query.p1);
        query6(gestor->reservas, gestor->voos, gestor->passageiros, gestor->last_query.p1, comando,
               out);
        break;
    }
    default:
        printf(RED "✗ Query anterior inválida.\n" RESET);
        break;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    gestor->last_query.last_ms = tempo_ms(t0, t1);

    fechar_saida(out, caminho_saida);
    printf(CYAN "Tempo de execução: %.2f ms\n" RESET, gestor->last_query.last_ms);
}

static void mostrar_ajuda(void)
{
    printf(BOLD "\nAjuda e Atalhos\n" RESET);
    printf("  - q1..q6: executa diretamente uma query\n");
    printf("  - stats: mostra estatísticas do dataset\n");
    printf("  - reload: recarrega o dataset\n");
    printf("  - history: mostra o histórico de comandos\n");
    printf("  - repeat / r: repete a última query\n");
    printf("  - clear: limpa o ecrã\n");
    printf("  - last: mostra a última query executada\n");
}

/**
 * @brief Executa o ciclo principal do modo interativo.
 *
 * Solicita o caminho do dataset, carrega os dados e apresenta
 * o menu de queries até o utilizador escolher sair.
 *
 * @param gestor Gestor interativo
 */
void gestor_interativo_executar(gestor_interativo_t *gestor)
{
    char caminho_dataset[BUFFER_SIZE];
    char opcao[10];

    // Banner
    limpar_ecra();
    printf(BOLD GREEN "\n");
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║                                                       ║\n");
    printf("║         SISTEMA DE GESTÃO DE VOOS - LI3 2025         ║\n");
    printf("║              Modo Interativo                          ║\n");
    printf("║                                                       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf(RESET "\n");

    // Solicitar caminho
    printf("Introduza o caminho dos ficheiros de dados\n");
    if (!ler_linha("(deixe vazio para usar default): ", caminho_dataset, sizeof(caminho_dataset),
                   0))
        copiar_dataset_padrao(caminho_dataset, sizeof(caminho_dataset));
    if (strlen(caminho_dataset) == 0)
        copiar_dataset_padrao(caminho_dataset, sizeof(caminho_dataset));

    // Carregar dataset
    carregar_dataset(gestor, caminho_dataset);

    // Loop principal
    while (1) {
        mostrar_menu();
        printf(BOLD "Escolha uma opção: " RESET);

        if (!fgets(opcao, sizeof(opcao), stdin))
            break;
        limpar_input(opcao);

        if (strlen(opcao) == 0)
            continue;

        int escolha = -1;
        if ((opcao[0] == 'q' || opcao[0] == 'Q') && isdigit((unsigned char)opcao[1])) {
            escolha = opcao[1] - '0';
        } else if (strcmp(opcao, "stats") == 0) {
            escolha = 7;
        } else if (strcmp(opcao, "reload") == 0) {
            escolha = 8;
        } else if (strcmp(opcao, "help") == 0) {
            escolha = 9;
        } else if (strcmp(opcao, "history") == 0 || strcmp(opcao, "hist") == 0) {
            escolha = 10;
        } else if (strcmp(opcao, "repeat") == 0 || strcmp(opcao, "r") == 0) {
            escolha = 12;
        } else if (strcmp(opcao, "last") == 0) {
            escolha = 11;
        } else if (strcmp(opcao, "clear") == 0) {
            escolha = 99;
        } else {
            escolha = atoi(opcao);
        }

        printf("\n");

        switch (escolha) {
        case 0:
            printf(YELLOW "Encerrando programa...\n" RESET);
            printf(GREEN "\n✓ Programa encerrado. Até breve!\n" RESET);
            return;

        case 1:
            executar_query1(gestor);
            break;

        case 2:
            executar_query2(gestor);
            break;

        case 3:
            executar_query3(gestor);
            break;

        case 4:
            executar_query4(gestor);
            break;

        case 5:
            executar_query5(gestor);
            break;

        case 6:
            executar_query6(gestor);
            break;

        case 7:
            mostrar_estatisticas(gestor);
            break;

        case 8: {
            char novo_caminho[BUFFER_SIZE];
            if (!ler_linha("Novo caminho do dataset: ", novo_caminho, sizeof(novo_caminho), 1)) {
                printf(RED "✗ Caminho inválido.\n" RESET);
                break;
            }
            if (!reiniciar_gestores(gestor)) {
                printf(RED "✗ Erro ao reiniciar gestores.\n" RESET);
                break;
            }
            carregar_dataset(gestor, novo_caminho);
            break;
        }

        case 9:
            mostrar_ajuda();
            break;

        case 10:
            history_show(gestor);
            break;

        case 11:
            mostrar_ultima_query(gestor);
            break;

        case 12:
            executar_ultima_query(gestor);
            break;

        case 99:
            limpar_ecra();
            break;

        default:
            printf(RED "✗ Opção inválida! Escolha entre 0-12.\n" RESET);
            break;
        }

        printf("\n" YELLOW "Pressione Enter para continuar..." RESET);
        getchar();
    }
}
