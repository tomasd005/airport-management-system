#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE 4096 /**< Tamanho máximo de linha lida */
#define COR_VERDE "\033[32m"
#define COR_VERMELHO "\033[31m"
#define COR_AMARELO "\033[33m"
#define COR_AZUL "\033[34m"
#define COR_RESET "\033[0m"

/**
 * @brief Remove espaços em branco do início e fim da string.
 *
 * @param str String a ser tratada.
 */
void trim_string(char *str)
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

/**
 * @brief Verifica se um arquivo existe no sistema.
 *
 * @param path Caminho do arquivo.
 * @return true se existe, false caso contrário.
 */
bool arquivo_existe(const char *path)
{
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

/**
 * @brief Conta as linhas de dados em um arquivo CSV, ignorando o header.
 *
 * Linhas vazias são ignoradas.
 *
 * @param filepath Caminho para o arquivo CSV.
 * @return Número de linhas de dados, ou -1 em caso de erro.
 */
int contar_linhas(const char *filepath)
{
    FILE *f = fopen(filepath, "r");
    if (!f)
        return -1;

    int count = 0;
    char linha[MAX_LINE];

    if (fgets(linha, sizeof(linha), f))
    {
        while (fgets(linha, sizeof(linha), f))
        {
            trim_string(linha);
            if (strlen(linha) > 0)
                count++;
        }
    }

    fclose(f);
    return count;
}

/**
 * @brief Testa falsos positivos nos ficheiros *_errors.csv.
 *
 * @param pasta_erros Caminho da pasta contendo os ficheiros *_errors.csv
 */
void testar_falsos_positivos(const char *pasta_erros)
{
    const char *tipos[] = {
        "airports_errors.csv",
        "aircrafts_errors.csv",
        "flights_errors.csv",
        "passengers_errors.csv",
        "reservations_errors.csv"};

    int total_falsos_positivos = 0;

    printf("\n%s╔════════════════════════════════════════════════════════╗%s\n",
           COR_AZUL, COR_RESET);
    printf("%s║     TESTE DE FALSOS POSITIVOS (Dataset SEM ERROS)     ║%s\n",
           COR_AZUL, COR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n\n",
           COR_AZUL, COR_RESET);

    for (int i = 0; i < 5; i++)
    {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", pasta_erros, tipos[i]);

        printf("  [%s]\n", tipos[i]);

        if (!arquivo_existe(path))
        {
            printf("    %sNÃO ENCONTRADO%s\n\n", COR_AMARELO, COR_RESET);
            continue;
        }

        int linhas = contar_linhas(path);

        if (linhas == 0)
        {
            printf("    %sOK - 0 erros (correto)%s\n\n", COR_VERDE, COR_RESET);
        }
        else
        {
            printf("    %sERRO - %d linhas%s\n", COR_VERMELHO, linhas, COR_RESET);
            total_falsos_positivos += linhas;

            FILE *f = fopen(path, "r");
            if (f)
            {
                char linha[MAX_LINE];
                fgets(linha, sizeof(linha), f);

                printf("    Primeiras linhas incorretamente marcadas:\n");
                for (int j = 0; j < 3 && fgets(linha, sizeof(linha), f); j++)
                {
                    trim_string(linha);
                    if (strlen(linha) > 80)
                        linha[80] = '\0';
                    printf("      %d: %s...\n", j + 1, linha);
                }
                printf("\n");
                fclose(f);
            }
        }
    }

    printf("%s═══════════════════════════════════════════════════════════%s\n",
           COR_AZUL, COR_RESET);

    if (total_falsos_positivos == 0)
    {
        printf("%s✓ PERFEITO: Nenhum falso positivo!%s\n", COR_VERDE, COR_RESET);
        printf("  Todas as validações estão corretas.\n");
        printf("  Podes submeter na plataforma.\n");
    }
    else
    {
        printf("%s✗ PROBLEMA: %d falsos positivos encontrados%s\n",
               COR_VERMELHO, total_falsos_positivos, COR_RESET);
        printf("  Estás a rejeitar dados válidos.\n\n");
        printf("  Causas comuns:\n");
        printf("  - Validar espaços ANTES de fazer trim\n");
        printf("  - Rejeitar datas futuras (voos futuros são válidos)\n");
        printf("  - Comparações case-sensitive incorrectas\n\n");
        printf("  Corrige as validações e testa novamente.\n");
    }

    printf("%s═══════════════════════════════════════════════════════════%s\n\n",
           COR_AZUL, COR_RESET);
}

/**
 * @brief Mostra instruções de uso do programa.
 *
 * @param prog Nome do programa.
 */
void print_usage(const char *prog)
{
    printf("\nUSO: %s <pasta_resultados>\n\n", prog);
    printf("WORKFLOW:\n\n");
    printf("  1. Executar com dataset SEM erros:\n");
    printf("     ./programa-principal sem_erros/ input.txt\n\n");
    printf("  2. Testar falsos positivos:\n");
    printf("     %s resultados/\n\n", prog);
    printf("  3. Se houver falsos positivos:\n");
    printf("     - Aplicar correções nos ficheiros validacao_*.c\n");
    printf("     - Recompilar: make clean && make\n");
    printf("     - Repetir teste\n\n");
    printf("  4. Quando não houver falsos positivos:\n");
    printf("     - Executar com dataset COM erros\n");
    printf("     - Submeter na plataforma\n\n");
    printf("NOTA:\n");
    printf("  Este programa só testa FALSOS POSITIVOS.\n");
    printf("  Para falsos negativos, precisas de debugging manual.\n\n");
}

/**
 * @brief Função principal do programa.
 *
 * @param argc Número de argumentos.
 * @param argv Vetor de argumentos.
 * @return 0 se sucesso, 1 se uso incorreto.
 */
int main(int argc, char *argv[])
{
    printf("\n%s╔════════════════════════════════════════════════════════╗%s\n",
           COR_AZUL, COR_RESET);
    printf("%s║          VALIDADOR DE CSVs DE ERROS - LI3              ║%s\n",
           COR_AZUL, COR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n",
           COR_AZUL, COR_RESET);

    if (argc != 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    testar_falsos_positivos(argv[1]);

    return 0;
}
/**
 * @file validador_falsos_positivos.c
 * @brief Programa para testar falsos positivos nos ficheiros *_errors.csv.
 *
 * Este utilitário verifica se, ao executar o programa principal com
 * datasets SEM erros, os ficheiros *_errors.csv estão corretamente
 * vazios (apenas com o header). Caso contrário, reporta os falsos positivos.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE 4096 /**< Tamanho máximo de linha lida */
#define COR_VERDE "\033[32m"
#define COR_VERMELHO "\033[31m"
#define COR_AMARELO "\033[33m"
#define COR_AZUL "\033[34m"
#define COR_RESET "\033[0m"

/**
 * @brief Remove espaços em branco do início e fim da string.
 *
 * @param str String a ser tratada.
 */
void trim_string(char *str)
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

/**
 * @brief Verifica se um arquivo existe no sistema.
 *
 * @param path Caminho do arquivo.
 * @return true se existe, false caso contrário.
 */
bool arquivo_existe(const char *path)
{
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

/**
 * @brief Conta as linhas de dados em um arquivo CSV, ignorando o header.
 *
 * Linhas vazias são ignoradas.
 *
 * @param filepath Caminho para o arquivo CSV.
 * @return Número de linhas de dados, ou -1 em caso de erro.
 */
int contar_linhas(const char *filepath)
{
    FILE *f = fopen(filepath, "r");
    if (!f)
        return -1;

    int count = 0;
    char linha[MAX_LINE];

    if (fgets(linha, sizeof(linha), f))
    {
        while (fgets(linha, sizeof(linha), f))
        {
            trim_string(linha);
            if (strlen(linha) > 0)
                count++;
        }
    }

    fclose(f);
    return count;
}

/**
 * @brief Testa falsos positivos nos ficheiros *_errors.csv.
 *
 * @param pasta_erros Caminho da pasta contendo os ficheiros *_errors.csv
 */
void testar_falsos_positivos(const char *pasta_erros)
{
    const char *tipos[] = {
        "airports_errors.csv",
        "aircrafts_errors.csv",
        "flights_errors.csv",
        "passengers_errors.csv",
        "reservations_errors.csv"};

    int total_falsos_positivos = 0;

    printf("\n%s╔════════════════════════════════════════════════════════╗%s\n",
           COR_AZUL, COR_RESET);
    printf("%s║     TESTE DE FALSOS POSITIVOS (Dataset SEM ERROS)     ║%s\n",
           COR_AZUL, COR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n\n",
           COR_AZUL, COR_RESET);

    for (int i = 0; i < 5; i++)
    {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", pasta_erros, tipos[i]);

        printf("  [%s]\n", tipos[i]);

        if (!arquivo_existe(path))
        {
            printf("    %sNÃO ENCONTRADO%s\n\n", COR_AMARELO, COR_RESET);
            continue;
        }

        int linhas = contar_linhas(path);

        if (linhas == 0)
        {
            printf("    %sOK - 0 erros (correto)%s\n\n", COR_VERDE, COR_RESET);
        }
        else
        {
            printf("    %sERRO - %d linhas%s\n", COR_VERMELHO, linhas, COR_RESET);
            total_falsos_positivos += linhas;

            FILE *f = fopen(path, "r");
            if (f)
            {
                char linha[MAX_LINE];
                fgets(linha, sizeof(linha), f);

                printf("    Primeiras linhas incorretamente marcadas:\n");
                for (int j = 0; j < 3 && fgets(linha, sizeof(linha), f); j++)
                {
                    trim_string(linha);
                    if (strlen(linha) > 80)
                        linha[80] = '\0';
                    printf("      %d: %s...\n", j + 1, linha);
                }
                printf("\n");
                fclose(f);
            }
        }
    }

    printf("%s═══════════════════════════════════════════════════════════%s\n",
           COR_AZUL, COR_RESET);

    if (total_falsos_positivos == 0)
    {
        printf("%s✓ PERFEITO: Nenhum falso positivo!%s\n", COR_VERDE, COR_RESET);
        printf("  Todas as validações estão corretas.\n");
        printf("  Podes submeter na plataforma.\n");
    }
    else
    {
        printf("%s✗ PROBLEMA: %d falsos positivos encontrados%s\n",
               COR_VERMELHO, total_falsos_positivos, COR_RESET);
        printf("  Estás a rejeitar dados válidos.\n\n");
        printf("  Causas comuns:\n");
        printf("  - Validar espaços ANTES de fazer trim\n");
        printf("  - Rejeitar datas futuras (voos futuros são válidos)\n");
        printf("  - Comparações case-sensitive incorrectas\n\n");
        printf("  Corrige as validações e testa novamente.\n");
    }

    printf("%s═══════════════════════════════════════════════════════════%s\n\n",
           COR_AZUL, COR_RESET);
}

/**
 * @brief Mostra instruções de uso do programa.
 *
 * @param prog Nome do programa.
 */
void print_usage(const char *prog)
{
    printf("\nUSO: %s <pasta_resultados>\n\n", prog);
    printf("WORKFLOW:\n\n");
    printf("  1. Executar com dataset SEM erros:\n");
    printf("     ./programa-principal sem_erros/ input.txt\n\n");
    printf("  2. Testar falsos positivos:\n");
    printf("     %s resultados/\n\n", prog);
    printf("  3. Se houver falsos positivos:\n");
    printf("     - Aplicar correções nos ficheiros validacao_*.c\n");
    printf("     - Recompilar: make clean && make\n");
    printf("     - Repetir teste\n\n");
    printf("  4. Quando não houver falsos positivos:\n");
    printf("     - Executar com dataset COM erros\n");
    printf("     - Submeter na plataforma\n\n");
    printf("NOTA:\n");
    printf("  Este programa só testa FALSOS POSITIVOS.\n");
    printf("  Para falsos negativos, precisas de debugging manual.\n\n");
}

/**
 * @brief Função principal do programa.
 *
 * @param argc Número de argumentos.
 * @param argv Vetor de argumentos.
 * @return 0 se sucesso, 1 se uso incorreto.
 */
int main(int argc, char *argv[])
{
    printf("\n%s╔════════════════════════════════════════════════════════╗%s\n",
           COR_AZUL, COR_RESET);
    printf("%s║          VALIDADOR DE CSVs DE ERROS - LI3              ║%s\n",
           COR_AZUL, COR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n",
           COR_AZUL, COR_RESET);

    if (argc != 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    testar_falsos_positivos(argv[1]);

    return 0;
}
