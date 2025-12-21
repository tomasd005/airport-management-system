#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "validacao_comum.h"
#include "validacao_aeroportos.h"
#include "validacao_avioes.h"
#include "validacao_voos.h"
#include "validacao_passageiros.h"
#include "validacao_reservas.h"
#include "utils.h"

#include "aeroportos.h"
#include "avioes.h"
#include "voos.h"
#include "passageiros.h"
#include "reservas.h"

#define COR_VERDE "\033[32m"
#define COR_VERMELHO "\033[31m"
#define COR_AMARELO "\033[33m"
#define COR_AZUL "\033[34m"
#define COR_RESET "\033[0m"

typedef struct
{
    int total;
    int passou;
    int falhou;
} Estatisticas;

void init_stats(Estatisticas *stats)
{
    stats->total = 0;
    stats->passou = 0;
    stats->falhou = 0;
}

void print_resultado(const char *teste, gboolean esperado, gboolean obtido, Estatisticas *stats)
{
    stats->total++;
    if (esperado == obtido)
    {
        stats->passou++;
        printf("  %s✓%s %s\n", COR_VERDE, COR_RESET, teste);
    }
    else
    {
        stats->falhou++;
        printf("  %s✗%s %s (esperado: %s, obtido: %s)\n",
               COR_VERMELHO, COR_RESET, teste,
               esperado ? "VÁLIDO" : "INVÁLIDO",
               obtido ? "VÁLIDO" : "INVÁLIDO");
    }
}

void print_secao(const char *nome)
{
    printf("\n%s═══════════════════════════════════════════════════════%s\n", COR_AZUL, COR_RESET);
    printf("%s  %s%s\n", COR_AZUL, nome, COR_RESET);
    printf("%s═══════════════════════════════════════════════════════%s\n\n", COR_AZUL, COR_RESET);
}

void print_stats(const char *categoria, Estatisticas *stats)
{
    double percentagem = (stats->total > 0) ? (stats->passou * 100.0 / stats->total) : 0;

    printf("\n%s───────────────────────────────────────────────────────%s\n", COR_AMARELO, COR_RESET);
    printf("  %s: ", categoria);

    if (percentagem == 100.0)
    {
        printf("%s%.1f%%%s (%d/%d) ✓\n", COR_VERDE, percentagem, COR_RESET, stats->passou, stats->total);
    }
    else if (percentagem >= 50.0)
    {
        printf("%s%.1f%%%s (%d/%d)\n", COR_AMARELO, percentagem, COR_RESET, stats->passou, stats->total);
    }
    else
    {
        printf("%s%.1f%%%s (%d/%d)\n", COR_VERMELHO, percentagem, COR_RESET, stats->passou, stats->total);
    }

    printf("%s───────────────────────────────────────────────────────%s\n", COR_AMARELO, COR_RESET);
}

void testar_validacao_ano(Estatisticas *stats)
{
    print_secao("TESTE: Validação de Ano");

    int ano;

    print_resultado("Ano 2020", TRUE, validacao_ano("2020", &ano) && ano == 2020, stats);
    print_resultado("Ano 1900", TRUE, validacao_ano("1900", &ano) && ano == 1900, stats);
    print_resultado("Ano 2025", TRUE, validacao_ano("2025", &ano) && ano == 2025, stats);
    print_resultado("Ano 2000", TRUE, validacao_ano("2000", &ano) && ano == 2000, stats);

    print_resultado("Ano com 3 dígitos", FALSE, validacao_ano("202", NULL), stats);
    print_resultado("Ano com 5 dígitos", FALSE, validacao_ano("20200", NULL), stats);
    print_resultado("Ano < 1900", FALSE, validacao_ano("1899", NULL), stats);
    print_resultado("Ano > 2025", FALSE, validacao_ano("2026", NULL), stats);
    print_resultado("Ano com letras", FALSE, validacao_ano("20a0", NULL), stats);
    print_resultado("Ano NULL", FALSE, validacao_ano(NULL, NULL), stats);
    print_resultado("Ano com espaços", FALSE, validacao_ano("20 20", NULL), stats);
}

void testar_validacao_data(Estatisticas *stats)
{
    print_secao("TESTE: Validação de Data");

    print_resultado("Data 2020-01-15", TRUE, validacao_data("2020-01-15"), stats);
    print_resultado("Data 2025-09-30", TRUE, validacao_data("2025-09-30"), stats);
    print_resultado("Data 1900-01-01", TRUE, validacao_data("1900-01-01"), stats);

    print_resultado("Data formato errado (sem -)", FALSE, validacao_data("20200115"), stats);
    print_resultado("Data mês inválido", FALSE, validacao_data("2020-13-15"), stats);
    print_resultado("Data dia inválido", FALSE, validacao_data("2020-01-32"), stats);
    print_resultado("Data futuro", FALSE, validacao_data("2025-12-31"), stats);
    print_resultado("Data NULL", FALSE, validacao_data(NULL), stats);
    print_resultado("Data com espaços extras", FALSE, validacao_data("2020 01-15"), stats);
    print_resultado("Data tamanho errado", FALSE, validacao_data("2020-1-15"), stats);
}

void testar_validacao_datetime(Estatisticas *stats)
{
    print_secao("TESTE: Validação de DateTime");

    print_resultado("DateTime 2020-01-15 10:30", TRUE, validacao_datetime("2020-01-15 10:30"), stats);
    print_resultado("DateTime 2024-01-01 00:00", TRUE, validacao_datetime("2024-01-01 00:00"), stats);
    print_resultado("DateTime 2020-12-31 23:59", TRUE, validacao_datetime("2020-12-31 23:59"), stats);

    print_resultado("DateTime sem espaço", FALSE, validacao_datetime("2020-01-1510:30"), stats);
    print_resultado("DateTime hora > 23", FALSE, validacao_datetime("2020-01-15 24:00"), stats);
    print_resultado("DateTime minuto > 59", FALSE, validacao_datetime("2020-01-15 10:60"), stats);
    print_resultado("DateTime NULL", FALSE, validacao_datetime(NULL), stats);
    print_resultado("DateTime tamanho errado", FALSE, validacao_datetime("2020-01-15 10:3"), stats);
}

void testar_validacao_flight_id(Estatisticas *stats)
{
    print_secao("TESTE: Validação de Flight ID");

    print_resultado("FlightID TP12345", TRUE, validacao_flight_id("TP12345"), stats);
    print_resultado("FlightID AA00000", TRUE, validacao_flight_id("AA00000"), stats);
    print_resultado("FlightID ZZ99999", TRUE, validacao_flight_id("ZZ99999"), stats);

    print_resultado("FlightID minúsculas", FALSE, validacao_flight_id("tp12345"), stats);
    print_resultado("FlightID 1 letra", FALSE, validacao_flight_id("T123456"), stats);
    print_resultado("FlightID letras no número", FALSE, validacao_flight_id("TP1234A"), stats);
    print_resultado("FlightID tamanho errado", FALSE, validacao_flight_id("TP123"), stats);
    print_resultado("FlightID NULL", FALSE, validacao_flight_id(NULL), stats);
    print_resultado("FlightID com espaços", FALSE, validacao_flight_id("TP 1234"), stats);
}

void testar_validacao_coordenadas(Estatisticas *stats)
{
    print_secao("TESTE: Validação de Coordenadas");

    double lat, lon;

    print_resultado("Latitude 45.5", TRUE, validacao_coordenada("45.5", TRUE, &lat), stats);
    print_resultado("Latitude -89.9", TRUE, validacao_coordenada("-89.9", TRUE, &lat), stats);
    print_resultado("Longitude 179.5", TRUE, validacao_coordenada("179.5", FALSE, &lon), stats);
    print_resultado("Longitude -180.0", TRUE, validacao_coordenada("-180.0", FALSE, &lon), stats);

    print_resultado("Latitude > 90", FALSE, validacao_coordenada("91.0", TRUE, NULL), stats);
    print_resultado("Latitude < -90", FALSE, validacao_coordenada("-91.0", TRUE, NULL), stats);
    print_resultado("Longitude > 180", FALSE, validacao_coordenada("181.0", FALSE, NULL), stats);
    print_resultado("Longitude < -180", FALSE, validacao_coordenada("-181.0", FALSE, NULL), stats);
    print_resultado("Coordenada NULL", FALSE, validacao_coordenada(NULL, TRUE, NULL), stats);
    print_resultado("Coordenada com letras", FALSE, validacao_coordenada("45.5a", TRUE, NULL), stats);
}

void testar_validacao_inteiro_positivo(Estatisticas *stats)
{
    print_secao("TESTE: Validação de Inteiro Positivo");

    int val;

    print_resultado("Número 1", TRUE, validacao_inteiro_positivo("1", &val), stats);
    print_resultado("Número 12345", TRUE, validacao_inteiro_positivo("12345", &val), stats);
    print_resultado("Número 999999", TRUE, validacao_inteiro_positivo("999999", &val), stats);

    print_resultado("Número 0", FALSE, validacao_inteiro_positivo("0", NULL), stats);
    print_resultado("Número negativo", FALSE, validacao_inteiro_positivo("-1", NULL), stats);
    print_resultado("Número com letras", FALSE, validacao_inteiro_positivo("123a", NULL), stats);
    print_resultado("Número NULL", FALSE, validacao_inteiro_positivo(NULL, NULL), stats);
    print_resultado("Número com espaços", FALSE, validacao_inteiro_positivo("12 34", NULL), stats);
    print_resultado("String vazia", FALSE, validacao_inteiro_positivo("", NULL), stats);
}

void testar_validacao_aeroportos(Estatisticas *stats)
{
    print_secao("TESTE: Validação de Aeroportos");

    char col0[] = "LIS";
    char col1[] = "Lisbon Airport";
    char col2[] = "Lisbon";
    char col3[] = "Portugal";
    char col4[] = "38.7742";
    char col5[] = "-9.1342";
    char col6[] = "123";
    char col7[] = "large_airport";
    char *valido[] = {col0, col1, col2, col3, col4, col5, col6, col7, NULL};
    gpointer resultado = valida_aeroporto(valido);
    print_resultado("Aeroporto válido completo", TRUE, resultado != NULL, stats);
    if (resultado)
        aeroporto_destruir(resultado);

    char c1_0[] = "lis";
    char c1_1[] = "Airport";
    char c1_2[] = "City";
    char c1_3[] = "Country";
    char c1_4[] = "0";
    char c1_5[] = "0";
    char c1_6[] = "";
    char c1_7[] = "large_airport";
    char *codigo_minusculo[] = {c1_0, c1_1, c1_2, c1_3, c1_4, c1_5, c1_6, c1_7, NULL};
    resultado = valida_aeroporto(codigo_minusculo);
    print_resultado("Código minúsculas", FALSE, resultado != NULL, stats);
    if (resultado)
        aeroporto_destruir(resultado);

    char c2_0[] = "LI";
    char c2_1[] = "Airport";
    char c2_2[] = "City";
    char c2_3[] = "Country";
    char c2_4[] = "0";
    char c2_5[] = "0";
    char c2_6[] = "";
    char c2_7[] = "large_airport";
    char *codigo_curto[] = {c2_0, c2_1, c2_2, c2_3, c2_4, c2_5, c2_6, c2_7, NULL};
    resultado = valida_aeroporto(codigo_curto);
    print_resultado("Código 2 letras", FALSE, resultado != NULL, stats);
    if (resultado)
        aeroporto_destruir(resultado);

    char c3_0[] = "LIS";
    char c3_1[] = "Airport";
    char c3_2[] = "City";
    char c3_3[] = "Country";
    char c3_4[] = "0";
    char c3_5[] = "0";
    char c3_6[] = "";
    char c3_7[] = "invalid_type";
    char *tipo_invalido[] = {c3_0, c3_1, c3_2, c3_3, c3_4, c3_5, c3_6, c3_7, NULL};
    resultado = valida_aeroporto(tipo_invalido);
    print_resultado("Tipo de aeroporto inválido", FALSE, resultado != NULL, stats);
    if (resultado)
        aeroporto_destruir(resultado);
}

void testar_validacao_avioes(Estatisticas *stats)
{
    print_secao("TESTE: Validação de Aviões");

    char a1_0[] = "A320";
    char a1_1[] = "Airbus";
    char a1_2[] = "A320-200";
    char a1_3[] = "2020";
    char a1_4[] = "180";
    char a1_5[] = "6000";
    char *valido[] = {a1_0, a1_1, a1_2, a1_3, a1_4, a1_5, NULL};
    aviao_t *resultado = valida_aviao(valido);
    print_resultado("Avião válido completo", TRUE, resultado != NULL, stats);
    if (resultado)
        aviao_destruir(resultado);

    char a2_0[] = "A320";
    char a2_1[] = "Airbus";
    char a2_2[] = "A320-200";
    char a2_3[] = "";
    char a2_4[] = "180";
    char a2_5[] = "6000";
    char *sem_ano[] = {a2_0, a2_1, a2_2, a2_3, a2_4, a2_5, NULL};
    resultado = valida_aviao(sem_ano);
    print_resultado("Avião sem ano (válido)", TRUE, resultado != NULL, stats);
    if (resultado)
        aviao_destruir(resultado);

    char a3_0[] = "A320";
    char a3_1[] = "Airbus";
    char a3_2[] = "A320-200";
    char a3_3[] = "2020";
    char a3_4[] = "0";
    char a3_5[] = "6000";
    char *cap_zero[] = {a3_0, a3_1, a3_2, a3_3, a3_4, a3_5, NULL};
    resultado = valida_aviao(cap_zero);
    print_resultado("Capacidade zero", FALSE, resultado != NULL, stats);
    if (resultado)
        aviao_destruir(resultado);
}

void testar_validacao_voos(Estatisticas *stats)
{
    print_secao("TESTE: Validação de Voos");

    char v1_0[] = "TP12345";
    char v1_1[] = "2025-09-15 10:00";
    char v1_2[] = "2025-09-15 10:05";
    char v1_3[] = "2025-09-15 12:00";
    char v1_4[] = "2025-09-15 12:10";
    char v1_5[] = "A1";
    char v1_6[] = "On Time";
    char v1_7[] = "LIS";
    char v1_8[] = "OPO";
    char v1_9[] = "A320";
    char v1_10[] = "TAP";
    char v1_11[] = "url";
    char *valido[] = {v1_0, v1_1, v1_2, v1_3, v1_4, v1_5, v1_6, v1_7, v1_8, v1_9, v1_10, v1_11, NULL};
    voo_t *resultado = valida_voo(valido);
    print_resultado("Voo válido On Time", TRUE, resultado != NULL, stats);
    if (resultado)
        voo_destruir(resultado);

    char v2_0[] = "TP12345";
    char v2_1[] = "2025-09-15 10:00";
    char v2_2[] = "N/A";
    char v2_3[] = "2025-09-15 12:00";
    char v2_4[] = "N/A";
    char v2_5[] = "A1";
    char v2_6[] = "On Time";
    char v2_7[] = "LIS";
    char v2_8[] = "LIS";
    char v2_9[] = "A320";
    char v2_10[] = "TAP";
    char v2_11[] = "url";
    char *mesma_cidade[] = {v2_0, v2_1, v2_2, v2_3, v2_4, v2_5, v2_6, v2_7, v2_8, v2_9, v2_10, v2_11, NULL};
    resultado = valida_voo(mesma_cidade);
    print_resultado("Origem = Destino", FALSE, resultado != NULL, stats);
    if (resultado)
        voo_destruir(resultado);
}

void testar_validacao_passageiros(Estatisticas *stats)
{
    print_secao("TESTE: Validação de Passageiros");

    char p1_0[] = "123456789";
    char p1_1[] = "John";
    char p1_2[] = "Doe";
    char p1_3[] = "1990-01-15";
    char p1_4[] = "PT";
    char p1_5[] = "M";
    char p1_6[] = "john@example.com";
    char p1_7[] = "+351912345678";
    char p1_8[] = "Rua X";
    char p1_9[] = "photo.jpg";
    char *valido[] = {p1_0, p1_1, p1_2, p1_3, p1_4, p1_5, p1_6, p1_7, p1_8, p1_9, NULL};
    passageiro_t *resultado = valida_passageiro(valido);
    print_resultado("Passageiro válido completo", TRUE, resultado != NULL, stats);
    if (resultado)
        passageiro_destruir(resultado);

    char p2_0[] = "123456789";
    char p2_1[] = "John";
    char p2_2[] = "Doe";
    char p2_3[] = "1990-01-15";
    char p2_4[] = "PT";
    char p2_5[] = "M";
    char p2_6[] = "johnexample.com";
    char p2_7[] = "+351912345678";
    char p2_8[] = "Rua X";
    char p2_9[] = "photo.jpg";
    char *email_invalido[] = {p2_0, p2_1, p2_2, p2_3, p2_4, p2_5, p2_6, p2_7, p2_8, p2_9, NULL};
    resultado = valida_passageiro(email_invalido);
    print_resultado("Email sem @", FALSE, resultado != NULL, stats);
    if (resultado)
        passageiro_destruir(resultado);
}

int main(void)
{
    Estatisticas stats_comum, stats_aeroportos, stats_avioes, stats_voos, stats_passageiros;
    Estatisticas total;

    init_stats(&stats_comum);
    init_stats(&stats_aeroportos);
    init_stats(&stats_avioes);
    init_stats(&stats_voos);
    init_stats(&stats_passageiros);
    init_stats(&total);

    printf("\n");
    printf("%s╔═══════════════════════════════════════════════════════╗%s\n", COR_AZUL, COR_RESET);
    printf("%s║         SISTEMA DE TESTES DE VALIDAÇÃO                ║%s\n", COR_AZUL, COR_RESET);
    printf("%s╚═══════════════════════════════════════════════════════╝%s\n", COR_AZUL, COR_RESET);

    testar_validacao_ano(&stats_comum);
    testar_validacao_data(&stats_comum);
    testar_validacao_datetime(&stats_comum);
    testar_validacao_flight_id(&stats_comum);
    testar_validacao_coordenadas(&stats_comum);
    testar_validacao_inteiro_positivo(&stats_comum);

    testar_validacao_aeroportos(&stats_aeroportos);
    testar_validacao_avioes(&stats_avioes);
    testar_validacao_voos(&stats_voos);
    testar_validacao_passageiros(&stats_passageiros);

    total.total = stats_comum.total + stats_aeroportos.total + stats_avioes.total +
                  stats_voos.total + stats_passageiros.total;
    total.passou = stats_comum.passou + stats_aeroportos.passou + stats_avioes.passou +
                   stats_voos.passou + stats_passageiros.passou;
    total.falhou = total.total - total.passou;

    printf("\n\n");
    printf("%s╔═══════════════════════════════════════════════════════╗%s\n", COR_AZUL, COR_RESET);
    printf("%s║                    RESUMO FINAL                       ║%s\n", COR_AZUL, COR_RESET);
    printf("%s╚═══════════════════════════════════════════════════════╝%s\n", COR_AZUL, COR_RESET);

    print_stats("Validação Comum", &stats_comum);
    print_stats("Validação Aeroportos", &stats_aeroportos);
    print_stats("Validação Aviões", &stats_avioes);
    print_stats("Validação Voos", &stats_voos);
    print_stats("Validação Passageiros", &stats_passageiros);

    printf("\n");
    printf("%s╔═══════════════════════════════════════════════════════╗%s\n", COR_AMARELO, COR_RESET);
    printf("%s║                    TOTAL GERAL                        ║%s\n", COR_AMARELO, COR_RESET);
    printf("%s╚═══════════════════════════════════════════════════════╝%s\n", COR_AMARELO, COR_RESET);

    double percentagem_total = (total.passou * 100.0 / total.total);

    printf("\n");
    printf("  Testes executados: %d\n", total.total);
    printf("  Testes aprovados:  %s%d%s\n", COR_VERDE, total.passou, COR_RESET);
    printf("  Testes falhados:   %s%d%s\n", COR_VERMELHO, total.falhou, COR_RESET);
    printf("\n");

    if (percentagem_total == 100.0)
    {
        printf("  %s┌─────────────────────────────────────────────────────┐%s\n", COR_VERDE, COR_RESET);
        printf("  %s│  SUCESSO: %.1f%% dos testes passaram! ✓            │%s\n", COR_VERDE, percentagem_total, COR_RESET);
        printf("  %s└─────────────────────────────────────────────────────┘%s\n", COR_VERDE, COR_RESET);
    }
    else if (percentagem_total >= 80.0)
    {
        printf("  %s┌─────────────────────────────────────────────────────┐%s\n", COR_AMARELO, COR_RESET);
        printf("  %s│  BOM: %.1f%% dos testes passaram                    │%s\n", COR_AMARELO, percentagem_total, COR_RESET);
        printf("  %s└─────────────────────────────────────────────────────┘%s\n", COR_AMARELO, COR_RESET);
    }
    else
    {
        printf("  %s┌─────────────────────────────────────────────────────┐%s\n", COR_VERMELHO, COR_RESET);
        printf("  %s│  ATENÇÃO: %.1f%% - Revisar testes falhados          │%s\n", COR_VERMELHO, percentagem_total, COR_RESET);
        printf("  %s└─────────────────────────────────────────────────────┘%s\n", COR_VERMELHO, COR_RESET);
    }

    printf("\n");

    return (total.falhou == 0) ? 0 : 1;
}