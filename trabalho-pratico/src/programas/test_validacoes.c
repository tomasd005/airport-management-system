#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE 2048
#define MAX_ERRORS 10000

#define COR_VERDE "\033[32m"
#define COR_VERMELHO "\033[31m"
#define COR_AMARELO "\033[33m"
#define COR_AZUL "\033[34m"
#define COR_CIANO "\033[36m"
#define COR_RESET "\033[0m"

typedef struct {
    char linha[MAX_LINE];
    int linha_num;
} ErroCSV;

typedef struct {
    int total_linhas;
    int erros_detectados;
    int falsos_positivos;
    int falsos_negativos;
    char tipo[32];
} EstatisticasCSV;

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

void trim_string(char *str) {
    if (!str) return;
    
    // Remove trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    
    // Remove leading whitespace
    char *start = str;
    while (*start && isspace((unsigned char)*start))
        start++;
    
    if (start != str)
        memmove(str, start, strlen(start) + 1);
}

bool arquivo_existe(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

int contar_linhas_arquivo(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) return -1;
    
    int count = 0;
    char linha[MAX_LINE];
    
    // Pular header
    if (fgets(linha, sizeof(linha), f)) {
        while (fgets(linha, sizeof(linha), f)) {
            trim_string(linha);
            if (strlen(linha) > 0)
                count++;
        }
    }
    
    fclose(f);
    return count;
}

// ============================================================================
// ANÁLISE DE FALSOS POSITIVOS
// ============================================================================

void analisar_falsos_positivos(const char *dataset_limpo, 
                                const char *pasta_erros,
                                EstatisticasCSV *stats) {
    (void)dataset_limpo;  // Informativo - não usado na análise de CSVs
    (void)stats;          // Reservado para estatísticas futuras
    
    printf("\n%s╔════════════════════════════════════════════════════════╗%s\n", COR_AZUL, COR_RESET);
    printf("%s║     ANÁLISE DE FALSOS POSITIVOS (Dataset Sem Erros)   ║%s\n", COR_AZUL, COR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n", COR_AZUL, COR_RESET);
    
    const char *tipos[] = {
        "airports_errors.csv",
        "aircrafts_errors.csv", 
        "flights_errors.csv",
        "passengers_errors.csv",
        "reservations_errors.csv"
    };
    
    int total_falsos_positivos = 0;
    
    for (int i = 0; i < 5; i++) {
        char path_erro[512];
        snprintf(path_erro, sizeof(path_erro), "%s/%s", pasta_erros, tipos[i]);
        
        int linhas_erro = contar_linhas_arquivo(path_erro);
        
        if (linhas_erro < 0) {
            printf("\n%s[%s]%s\n", COR_AMARELO, tipos[i], COR_RESET);
            printf("  %s⚠ Ficheiro não encontrado%s\n", COR_AMARELO, COR_RESET);
            continue;
        }
        
        printf("\n%s[%s]%s\n", COR_CIANO, tipos[i], COR_RESET);
        
        if (linhas_erro == 0) {
            printf("  %s✓ Nenhum erro detectado (correto!)%s\n", COR_VERDE, COR_RESET);
        } else {
            printf("  %s✗ %d linhas no ficheiro de erros%s\n", 
                   COR_VERMELHO, linhas_erro, COR_RESET);
            printf("  %s⚠ FALSOS POSITIVOS: Dados válidos marcados como errados%s\n",
                   COR_VERMELHO, COR_RESET);
            total_falsos_positivos += linhas_erro;
            
            // Mostrar primeiros erros para debug
            FILE *f = fopen(path_erro, "r");
            if (f) {
                char linha[MAX_LINE];
                fgets(linha, sizeof(linha), f); // skip header
                
                printf("\n  Primeiras 3 linhas detectadas como erro:\n");
                for (int j = 0; j < 3 && fgets(linha, sizeof(linha), f); j++) {
                    trim_string(linha);
                    if (strlen(linha) > 0) {
                        printf("    %s%d: %.80s...%s\n", 
                               COR_AMARELO, j+1, linha, COR_RESET);
                    }
                }
                fclose(f);
            }
        }
    }
    
    printf("\n%s─────────────────────────────────────────────────────────%s\n", 
           COR_AMARELO, COR_RESET);
    if (total_falsos_positivos == 0) {
        printf("%s✓ EXCELENTE: Nenhum falso positivo detectado!%s\n", 
               COR_VERDE, COR_RESET);
    } else {
        printf("%s⚠ TOTAL DE FALSOS POSITIVOS: %d%s\n", 
               COR_VERMELHO, total_falsos_positivos, COR_RESET);
        printf("  → Revise as validações que estão rejeitando dados válidos\n");
    }
    printf("%s─────────────────────────────────────────────────────────%s\n", 
           COR_AMARELO, COR_RESET);
}

// ============================================================================
// ANÁLISE DE FALSOS NEGATIVOS
// ============================================================================

typedef struct {
    char *nome_arquivo;
    int linha_original;
    char descricao[256];
} ErroEsperado;

void carregar_erros_esperados(const char *dataset_erros,
                               const char *tipo,
                               ErroEsperado **out_erros,
                               int *out_count) {
    (void)out_erros;  // Não usado nesta implementação simplificada
    
    // Esta função deve ler o CSV original do dataset com erros
    // e identificar quais linhas têm problemas conhecidos
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dataset_erros, tipo);
    
    FILE *f = fopen(path, "r");
    if (!f) {
        *out_count = 0;
        return;
    }
    
    // Implementação simplificada - na prática, você deve ter uma lista
    // de erros conhecidos por dataset
    *out_count = 0;
    fclose(f);
}

void analisar_falsos_negativos(const char *dataset_erros,
                                const char *pasta_erros_gerados,
                                EstatisticasCSV *stats) {
    (void)dataset_erros;        // Informativo - não usado na análise de CSVs
    (void)stats;                // Reservado para estatísticas futuras
    
    printf("\n%s╔════════════════════════════════════════════════════════╗%s\n", COR_AZUL, COR_RESET);
    printf("%s║      ANÁLISE DE FALSOS NEGATIVOS (Dataset com Erros)  ║%s\n", COR_AZUL, COR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n", COR_AZUL, COR_RESET);
    
    printf("\n%s⚠ NOTA:%s Esta análise requer conhecimento dos erros esperados\n", 
           COR_AMARELO, COR_RESET);
    printf("  no dataset. Você deve:\n\n");
    printf("  1. Documentar manualmente os erros conhecidos no dataset\n");
    printf("  2. Verificar se cada erro foi detectado nos CSVs gerados\n");
    printf("  3. Qualquer erro não detectado = falso negativo\n\n");
    
    const char *tipos[] = {
        "airports.csv",
        "aircrafts.csv", 
        "flights.csv",
        "passengers.csv",
        "reservations.csv"
    };
    
    const char *csv_erros[] = {
        "airports_errors.csv",
        "aircrafts_errors.csv", 
        "flights_errors.csv",
        "passengers_errors.csv",
        "reservations_errors.csv"
    };
    
    printf("%sEXEMPLO DE DEBUGGING MANUAL:%s\n\n", COR_CIANO, COR_RESET);
    
    for (int i = 0; i < 5; i++) {
        printf("%s[%s]%s\n", COR_CIANO, tipos[i], COR_RESET);
        
        char path_erro[512];
        snprintf(path_erro, sizeof(path_erro), "%s/%s", 
                 pasta_erros_gerados, csv_erros[i]);
        
        int erros_detectados = contar_linhas_arquivo(path_erro);
        
        printf("  • Erros detectados pelo programa: %d\n", 
               erros_detectados >= 0 ? erros_detectados : 0);
        printf("  • Erros conhecidos no dataset: %s[A DOCUMENTAR]%s\n",
               COR_AMARELO, COR_RESET);
        printf("  • Verificar manualmente se cada erro conhecido foi detectado\n\n");
    }
    
    printf("%s─────────────────────────────────────────────────────────%s\n", 
           COR_AMARELO, COR_RESET);
    printf("Para análise completa de falsos negativos:\n");
    printf("1. Crie ficheiro com lista de erros esperados\n");
    printf("2. Compare com erros detectados\n");
    printf("3. Diferença = falsos negativos\n");
    printf("%s─────────────────────────────────────────────────────────%s\n", 
           COR_AMARELO, COR_RESET);
}

// ============================================================================
// COMPARAÇÃO DE CSVs DE ERRO COM ESPERADOS
// ============================================================================

bool comparar_csvs_erro(const char *gerado, const char *esperado, int *linha_dif) {
    FILE *f1 = fopen(gerado, "r");
    FILE *f2 = fopen(esperado, "r");
    
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return false;
    }
    
    char l1[MAX_LINE], l2[MAX_LINE];
    int linha = 0;
    
    while (fgets(l1, sizeof(l1), f1) && fgets(l2, sizeof(l2), f2)) {
        linha++;
        trim_string(l1);
        trim_string(l2);
        
        if (strcmp(l1, l2) != 0) {
            *linha_dif = linha;
            fclose(f1);
            fclose(f2);
            return false;
        }
    }
    
    // Verificar se um ficheiro tem mais linhas que o outro
    if (fgets(l1, sizeof(l1), f1) || fgets(l2, sizeof(l2), f2)) {
        *linha_dif = linha + 1;
        fclose(f1);
        fclose(f2);
        return false;
    }
    
    fclose(f1);
    fclose(f2);
    return true;
}

void comparar_com_esperados(const char *pasta_gerados,
                            const char *pasta_esperados) {
    printf("\n%s╔════════════════════════════════════════════════════════╗%s\n", COR_AZUL, COR_RESET);
    printf("%s║     COMPARAÇÃO COM FICHEIROS ESPERADOS (Plataforma)   ║%s\n", COR_AZUL, COR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n", COR_AZUL, COR_RESET);
    
    const char *tipos[] = {
        "airports_errors.csv",
        "aircrafts_errors.csv", 
        "flights_errors.csv",
        "passengers_errors.csv",
        "reservations_errors.csv"
    };
    
    int total_ok = 0;
    int total_testados = 0;
    
    for (int i = 0; i < 5; i++) {
        char path_gerado[512], path_esperado[512];
        snprintf(path_gerado, sizeof(path_gerado), "%s/%s", pasta_gerados, tipos[i]);
        snprintf(path_esperado, sizeof(path_esperado), "%s/%s", pasta_esperados, tipos[i]);
        
        printf("\n%s[%s]%s\n", COR_CIANO, tipos[i], COR_RESET);
        
        if (!arquivo_existe(path_esperado)) {
            printf("  %s⚠ Ficheiro esperado não disponível%s\n", 
                   COR_AMARELO, COR_RESET);
            continue;
        }
        
        if (!arquivo_existe(path_gerado)) {
            printf("  %s✗ Ficheiro gerado não encontrado%s\n", 
                   COR_VERMELHO, COR_RESET);
            total_testados++;
            continue;
        }
        
        int linha_dif = 0;
        bool igual = comparar_csvs_erro(path_gerado, path_esperado, &linha_dif);
        
        total_testados++;
        
        if (igual) {
            printf("  %s✓ Idêntico ao esperado!%s\n", COR_VERDE, COR_RESET);
            total_ok++;
        } else {
            printf("  %s✗ Diferença encontrada na linha %d%s\n", 
                   COR_VERMELHO, linha_dif, COR_RESET);
            
            int linhas_gerado = contar_linhas_arquivo(path_gerado);
            int linhas_esperado = contar_linhas_arquivo(path_esperado);
            
            printf("    Linhas geradas: %d | Esperadas: %d\n", 
                   linhas_gerado, linhas_esperado);
        }
    }
    
    printf("\n%s─────────────────────────────────────────────────────────%s\n", 
           COR_AMARELO, COR_RESET);
    printf("Resultado: %d/%d ficheiros corretos\n", total_ok, total_testados);
    if (total_ok == total_testados && total_testados > 0) {
        printf("%s✓ PERFEITO: Todos os CSVs idênticos aos esperados!%s\n", 
               COR_VERDE, COR_RESET);
    } else {
        printf("%s⚠ Revise os ficheiros com diferenças%s\n", 
               COR_AMARELO, COR_RESET);
    }
    printf("%s─────────────────────────────────────────────────────────%s\n", 
           COR_AMARELO, COR_RESET);
}

// ============================================================================
// MAIN
// ============================================================================

void print_usage(const char *prog) {
    printf("Uso: %s <modo> [argumentos]\n\n", prog);
    printf("Modos disponíveis:\n\n");
    printf("  1. falsos-positivos <pasta_dataset_sem_erros> <pasta_erros_gerados>\n");
    printf("     Detecta dados válidos incorretamente marcados como erros\n\n");
    printf("  2. falsos-negativos <pasta_dataset_com_erros> <pasta_erros_gerados>\n");
    printf("     Ajuda a identificar erros não detectados\n\n");
    printf("  3. comparar <pasta_gerados> <pasta_esperados>\n");
    printf("     Compara CSVs gerados com os esperados pela plataforma\n\n");
    printf("  4. completo <pasta_dataset_sem_erros> <pasta_dataset_com_erros> <pasta_gerados> <pasta_esperados>\n");
    printf("     Executa todas as análises\n\n");
    printf("Exemplos:\n");
    printf("  %s falsos-positivos sem_erros resultados\n", prog);
    printf("  %s falsos-negativos com_erros resultados\n", prog);
    printf("  %s comparar resultados resultados-esperados\n", prog);
    printf("  %s completo sem_erros com_erros resultados resultados-esperados\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *modo = argv[1];
    
    printf("\n");
    printf("%s╔════════════════════════════════════════════════════════╗%s\n", 
           COR_AZUL, COR_RESET);
    printf("%s║      SISTEMA DE VALIDAÇÃO DE CSVs DE ERROS - LI3      ║%s\n", 
           COR_AZUL, COR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n", 
           COR_AZUL, COR_RESET);
    
    EstatisticasCSV stats = {0};
    
    if (strcmp(modo, "falsos-positivos") == 0) {
        if (argc != 4) {
            printf("Erro: falsos-positivos requer 2 argumentos\n");
            print_usage(argv[0]);
            return 1;
        }
        analisar_falsos_positivos(argv[2], argv[3], &stats);
        
    } else if (strcmp(modo, "falsos-negativos") == 0) {
        if (argc != 4) {
            printf("Erro: falsos-negativos requer 2 argumentos\n");
            print_usage(argv[0]);
            return 1;
        }
        analisar_falsos_negativos(argv[2], argv[3], &stats);
        
    } else if (strcmp(modo, "comparar") == 0) {
        if (argc != 4) {
            printf("Erro: comparar requer 2 argumentos\n");
            print_usage(argv[0]);
            return 1;
        }
        comparar_com_esperados(argv[2], argv[3]);
        
    } else if (strcmp(modo, "completo") == 0) {
        if (argc != 6) {
            printf("Erro: completo requer 4 argumentos\n");
            print_usage(argv[0]);
            return 1;
        }
        
        printf("\n%s▶ FASE 1: Análise de Falsos Positivos%s\n", 
               COR_CIANO, COR_RESET);
        analisar_falsos_positivos(argv[2], argv[4], &stats);
        
        printf("\n%s▶ FASE 2: Análise de Falsos Negativos%s\n", 
               COR_CIANO, COR_RESET);
        analisar_falsos_negativos(argv[3], argv[4], &stats);
        
        printf("\n%s▶ FASE 3: Comparação com Esperados%s\n", 
               COR_CIANO, COR_RESET);
        comparar_com_esperados(argv[4], argv[5]);
        
    } else {
        printf("Modo desconhecido: %s\n", modo);
        print_usage(argv[0]);
        return 1;
    }
    
    printf("\n");
    return 0;
}