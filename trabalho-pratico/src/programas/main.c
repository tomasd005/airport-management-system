#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/utils.h"
#include "../../include/gestores/gestor_aeroportos.h"
#include "../../include/gestores/gestor_avioes.h"
#include "../../include/gestores/gestor_voos.h"
#include "../../include/gestores/gestor_passageiros.h"
#include "../../include/gestores/gestor_reservas.h"
#include "../../include/queries/querie1.h"
#include "../../include/queries/querie2.h"
#include "../../include/queries/querie3.h"

static void trim_string(char *str)
{
    if (!str)
        return;

    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }

    char *start = str;
    while (*start && isspace((unsigned char)*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <pasta_datasets> <ficheiro_input>\n", argv[0]);
        return 1;
    }

    const char *pasta = argv[1];
    const char *ficheiro_input = argv[2];

    gestor_aeroportos_t *gestor_aeroportos = gestor_aeroportos_criar();
    gestor_avioes_t *gestor_avioes = gestor_avioes_criar();
    gestor_voos_t *gestor_voos = gestor_voos_criar();
    gestor_passageiros_t *gestor_passageiros = gestor_passageiros_criar();
    gestor_reservas_t *gestor_reservas = gestor_reservas_criar();

    char caminho_aeroportos[512];
    char caminho_avioes[512];
    char caminho_voos[512];
    char caminho_passageiros[512];
    char caminho_reservas[512];

    snprintf(caminho_aeroportos, sizeof(caminho_aeroportos), "%s/airports.csv", pasta);
    snprintf(caminho_avioes, sizeof(caminho_avioes), "%s/aircrafts.csv", pasta);
    snprintf(caminho_voos, sizeof(caminho_voos), "%s/flights.csv", pasta);
    snprintf(caminho_passageiros, sizeof(caminho_passageiros), "%s/passengers.csv", pasta);
    snprintf(caminho_reservas, sizeof(caminho_reservas), "%s/reservations.csv", pasta);

    gestor_aeroportos_carregar(gestor_aeroportos, caminho_aeroportos);
    gestor_avioes_carregar(gestor_avioes, caminho_avioes);
    gestor_voos_carregar(gestor_voos, caminho_voos);
    gestor_passageiros_carregar(gestor_passageiros, caminho_passageiros);
    gestor_reservas_carregar(gestor_reservas, caminho_reservas);

    FILE *input = fopen(ficheiro_input, "r");
    if (!input)
    {
        perror("Erro ao abrir ficheiro de input");
        gestor_aeroportos_destruir(gestor_aeroportos);
        gestor_avioes_destruir(gestor_avioes);
        gestor_voos_destruir(gestor_voos);
        gestor_passageiros_destruir(gestor_passageiros);
        gestor_reservas_destruir(gestor_reservas);

        return 1;
    }

    char linha[256];
    int contador = 1;

    while (fgets(linha, sizeof(linha), input))
    {
        size_t len = strlen(linha);
        while (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r'))
        {
            linha[len - 1] = '\0';
            len--;
        }

        if (len == 0)
            continue;

        char *p = linha;

        while (*p && isspace((unsigned char)*p))
            p++;

        if (!*p)
            continue;

        char *tipo_str = p;
        while (*p && !isspace((unsigned char)*p))
            p++;

        if (*p)
            *p++ = '\0';

        int tipo = atoi(tipo_str);

        char caminho_saida[256];
        snprintf(caminho_saida, sizeof(caminho_saida), "resultados/command%d_output.txt", contador);

        FILE *out = fopen(caminho_saida, "w");
        if (!out)
        {
            perror("Erro a criar ficheiro de output");
            contador++;
            continue;
        }

        if (tipo == 1)
        {
            while (*p && isspace((unsigned char)*p))
                p++;

            char *aeroporto = p;
            trim_string(aeroporto);

            query1(gestor_aeroportos, aeroporto, out);
        }
        else if (tipo == 2)
        {
            while (*p && isspace((unsigned char)*p))
                p++;

            char *n_str = p;
            while (*p && !isspace((unsigned char)*p))
                p++;

            if (*p)
                *p++ = '\0';

            int N = atoi(n_str);

            while (*p && isspace((unsigned char)*p))
                p++;

            char *fabricante = p;
            trim_string(fabricante);

            if (strlen(fabricante) == 0)
                fabricante = NULL;

            query2(gestor_avioes, gestor_voos, N, fabricante, out);
        }
        else if (tipo == 3)
        {
            while (*p && isspace((unsigned char)*p))
                p++;

            char *data_inicio = p;
            while (*p && !isspace((unsigned char)*p))
                p++;

            if (*p)
                *p++ = '\0';

            while (*p && isspace((unsigned char)*p))
                p++;

            char *data_fim = p;
            trim_string(data_fim);

            if (strlen(data_inicio) > 0 && strlen(data_fim) > 0)
            {
                query3(gestor_aeroportos, gestor_voos, data_inicio, data_fim, out);
            }
            else
            {
                fprintf(out, "\n");
            }
        }
        else
        {
            fprintf(out, "\n");
        }

        fclose(out);
        contador++;
    }

    fclose(input);

    gestor_aeroportos_destruir(gestor_aeroportos);
    gestor_avioes_destruir(gestor_avioes);
    gestor_voos_destruir(gestor_voos);

    return 0;
}