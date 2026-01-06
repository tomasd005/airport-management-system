#include "gestor_programa.h"
#include "gestor_queries.h"
#include <stdio.h>

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

    printf("Carregando dados...\n");

    snprintf(caminho, sizeof(caminho), "%s/airports.csv", pastaDados);
    gestor_aeroportos_carregar(gestor->aeroportos, caminho);
    printf("  Aeroportos carregados: %u\n", gestor_aeroportos_contar(gestor->aeroportos));

    snprintf(caminho, sizeof(caminho), "%s/aircrafts.csv", pastaDados);
    gestor_avioes_carregar(gestor->avioes, caminho);
    printf("  Avioes carregados: %u\n", gestor_avioes_contar(gestor->avioes));

    snprintf(caminho, sizeof(caminho), "%s/flights.csv", pastaDados);
    gestor_voos_carregar(gestor->voos, caminho);
    printf("  Voos carregados: %u\n", gestor_voos_contar(gestor->voos));

    snprintf(caminho, sizeof(caminho), "%s/passengers.csv", pastaDados);
    gestor_passageiros_carregar(gestor->passageiros, caminho);
    printf("  Passageiros carregados: %u\n", gestor_passageiros_numero(gestor->passageiros));

    snprintf(caminho, sizeof(caminho), "%s/reservations.csv", pastaDados);
    gestor_reservas_carregar_com_validacao(
        gestor->reservas,
        caminho,
        gestor->voos,
        gestor->passageiros);
    printf("  Reservas carregadas: %u\n", gestor_reservas_numero(gestor->reservas));

    printf("Processando queries...\n");

    gestor_queries_t *gestor_queries = gestor_queries_criar(
        gestor->aeroportos,
        gestor->avioes,
        gestor->voos,
        gestor->passageiros,
        gestor->reservas);

    gestor_queries_processar_ficheiro(gestor_queries, ficheiroInput);

    gestor_queries_destruir(gestor_queries);
}

void gestor_programa_destroi(GestorDePrograma *gestor)
{
    gestor_aeroportos_destruir(gestor->aeroportos);
    gestor_avioes_destruir(gestor->avioes);
    gestor_voos_destruir(gestor->voos);
    gestor_passageiros_destruir(gestor->passageiros);
    gestor_reservas_destruir(gestor->reservas);

    g_free(gestor);
}