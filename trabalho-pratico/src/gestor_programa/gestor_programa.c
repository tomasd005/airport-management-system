#include "gestor_programa.h"
#include "gestor_queries.h"
#include <stdio.h>
#include <sys/stat.h>

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

void gestor_programa_executa(GestorDePrograma *gestor, const char *pastaDados, const char *ficheiroInput)
{
    char caminho[512];

    // Criar pasta resultados se não existir
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
    gestor_voos_carregar(gestor->voos, caminho);
    unsigned int voos = gestor_voos_contar(gestor->voos);
    fprintf(stderr, "  Voos: %u\n", voos);

    snprintf(caminho, sizeof(caminho), "%s/passengers.csv", pastaDados);
    gestor_passageiros_carregar(gestor->passageiros, caminho);
    unsigned int passageiros = gestor_passageiros_numero(gestor->passageiros);
    fprintf(stderr, "  Passageiros: %u\n", passageiros);

    snprintf(caminho, sizeof(caminho), "%s/reservations.csv", pastaDados);
    gestor_reservas_carregar(gestor->reservas, caminho);
    unsigned int reservas = gestor_reservas_numero(gestor->reservas);
    fprintf(stderr, "  Reservas: %u\n", reservas);

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