#include "gestor_programa.h"
#include "gestor_queries.h"
#include "gestores/gestor_aeroportos.h"
#include "gestores/gestor_avioes.h"
#include "gestores/gestor_voos.h"
#include "gestores/gestor_passageiros.h"
#include "gestores/gestor_reservas.h"
#include <stdio.h>
#include <sys/stat.h>

/**
 * @struct gestor_programa
 * @brief Estrutura principal que agrega todos os gestores do sistema.
 *
 * Esta estrutura contém ponteiros para todos os gestores responsáveis
 * pelos diferentes tipos de dados (aeroportos, aviões, voos, passageiros
 * e reservas)
 */
struct gestor_programa
{
    gestor_aeroportos_t *aeroportos;
    gestor_avioes_t *avioes;
    gestor_voos_t *voos;
    gestor_passageiros_t *passageiros;
    gestor_reservas_t *reservas;
    gboolean modoEconomiaMemoria;
};

/**
 * @brief Cria e inicializa um novo gestor de programa.
 *
 * Aloca memória para o gestor principal e cria todos os gestores
 * internos necessários ao funcionamento do programa.
 *
 * @param modoEconomiaMemoria Indica se o programa deve usar menos memória
 * @return Ponteiro para o novo GestorDePrograma
 */
GestorDePrograma *gestor_programa_novo(gboolean modoEconomiaMemoria)
{
    GestorDePrograma *g = g_new0(GestorDePrograma, 1);
    g->modoEconomiaMemoria = modoEconomiaMemoria;

    g->aeroportos = gestor_aeroportos_criar();
    g->avioes = gestor_avioes_criar();
    g->voos = gestor_voos_criar();
    g->passageiros = gestor_passageiros_criar();
    g->reservas = gestor_reservas_criar();

    return g;
}

/**
 * @brief Executa o fluxo principal do programa.
 *
 * Esta função carrega todos os dados a partir dos ficheiros CSV,
 * valida a consistência da informação, prepara as estruturas
 * necessárias e processa as queries presentes no ficheiro de input.
 *
 * @param gestor Ponteiro para o gestor de programa
 * @param pastaDados Caminho para a pasta que contém os ficheiros CSV
 * @param ficheiroInput Caminho para o ficheiro de queries
 */
void gestor_programa_executa(GestorDePrograma *gestor, const char *pastaDados, const char *ficheiroInput)
{
    char caminho[512];

    /* Criar pasta de resultados caso não exista */
    mkdir("resultados", 0755);

    fprintf(stderr, "Carregando dados de: %s\n", pastaDados);

    snprintf(caminho, sizeof(caminho), "%s/airports.csv", pastaDados);
    gestor_aeroportos_carregar(gestor->aeroportos, caminho);
    unsigned int aeroportos = gestor_aeroportos_contar(gestor->aeroportos);
    fprintf(stderr, "  Aeroportos: %u\n", aeroportos);

    snprintf(caminho, sizeof(caminho), "%s/aircrafts.csv", pastaDados);
    gestor_avioes_carregar(gestor->avioes, caminho);
    unsigned int avioes = gestor_avioes_contar(gestor->avioes);
    fprintf(stderr, "  Avioes: %u\n", avioes);

    snprintf(caminho, sizeof(caminho), "%s/flights.csv", pastaDados);
    gestor_voos_carregar_com_validacao(gestor->voos, caminho, gestor->avioes);
    unsigned int voos = gestor_voos_contar(gestor->voos);
    fprintf(stderr, "  Voos: %u\n", voos);

    snprintf(caminho, sizeof(caminho), "%s/passengers.csv", pastaDados);
    gestor_passageiros_carregar(gestor->passageiros, caminho);
    unsigned int passageiros = gestor_passageiros_numero(gestor->passageiros);
    fprintf(stderr, "  Passageiros: %u\n", passageiros);

    snprintf(caminho, sizeof(caminho), "%s/reservations.csv", pastaDados);
    gestor_reservas_carregar_com_validacao(
        gestor->reservas, caminho,
        gestor->voos, gestor->passageiros);
    unsigned int reservas = gestor_reservas_numero(gestor->reservas);
    fprintf(stderr, "  Reservas: %u\n", reservas);

    /* Pós-processamento dos dados */
    gestor_reservas_finalizar(gestor->reservas);
    {
        GHashTable *docs_top10 = g_hash_table_new(g_direct_hash, g_direct_equal);
        gestor_reservas_coletar_docs_top10(gestor->reservas, docs_top10);
        gestor_passageiros_carregar_detalhes(gestor->passageiros, docs_top10);
        g_hash_table_destroy(docs_top10);
    }
    gestor_voos_atualizar_contagens_aeroportos(gestor->voos, gestor->aeroportos);
    gestor_voos_preparar_q3(gestor->voos);
    gestor_voos_descartar_tabela(gestor->voos);

    if (aeroportos == 0 || voos == 0 || passageiros == 0)
    {
        fprintf(stderr, "ERRO: Dados não carregados corretamente!\n");
        return;
    }

    fprintf(stderr, "Processando queries de: %s\n", ficheiroInput);

    gestor_queries_t *gestor_queries = gestor_queries_criar(
        gestor->aeroportos,
        gestor->avioes,
        gestor->voos,
        gestor->passageiros,
        gestor->reservas);

    gestor_queries_processar_ficheiro(gestor_queries, ficheiroInput);
    gestor_queries_destruir(gestor_queries);

    fprintf(stderr, "Processamento concluído!\n");
}

/**
 * @brief Liberta toda a memória associada ao gestor de programa.
 *
 * Destrói todos os gestores internos e liberta a estrutura principal.
 *
 * @param gestor Ponteiro para o gestor de programa a destruir
 */
void gestor_programa_destroi(GestorDePrograma *gestor)
{
    if (!gestor)
        return;

    gestor_aeroportos_destruir(gestor->aeroportos);
    gestor_avioes_destruir(gestor->avioes);
    gestor_voos_destruir(gestor->voos);
    gestor_passageiros_destruir(gestor->passageiros);
    gestor_reservas_destruir(gestor->reservas);

    g_free(gestor);
}
