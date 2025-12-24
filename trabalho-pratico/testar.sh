#!/bin/bash

# Script de Testes Automatizado - LI3
# Replica o processo de validação da plataforma

set -e  # Para em caso de erro

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configurações (ajuste conforme necessário)
PROGRAMA_PRINCIPAL="./programa-principal"
DATASET_LIMPO="dataset_limpo"
DATASET_ERROS="dataset_erros"
DATASET_NORMAL="dataset"
INPUT_FILE="inputs"
RESULTADOS="resultados"
ESPERADOS="resultados_esperados"

# ============================================================================
# FUNÇÕES AUXILIARES
# ============================================================================

print_header() {
    echo -e "${BLUE}╔════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║  $1${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════╝${NC}"
}

print_section() {
    echo ""
    echo -e "${CYAN}▶ $1${NC}"
    echo -e "${CYAN}────────────────────────────────────────────────────────${NC}"
}

print_success() {
    echo -e "${GREEN} $1${NC}"
}

print_error() {
    echo -e "${RED}$1${NC}"
}

print_warning() {
    echo -e "${YELLOW} $1${NC}"
}

check_file() {
    if [ ! -f "$1" ]; then
        print_error "Ficheiro não encontrado: $1"
        return 1
    fi
    return 0
}

check_dir() {
    if [ ! -d "$1" ]; then
        print_error "Diretório não encontrado: $1"
        return 1
    fi
    return 0
}

# ============================================================================
# COMPILAÇÃO
# ============================================================================

compilar() {
    print_section "Compilando programas de teste"
    
    if [ -f "Makefile.testes" ]; then
        make -f Makefile.testes all
    elif [ -f "Makefile" ]; then
        make all
    else
        print_warning "Makefile não encontrado, compilando manualmente..."
        gcc -o teste teste.c -lm 2>/dev/null || print_error "Falha ao compilar teste.c"
        gcc -o teste_csv teste_validacao_completo.c 2>/dev/null || print_error "Falha ao compilar teste_csv"
    fi
    
    print_success "Compilação concluída"
}

# ============================================================================
# TESTES
# ============================================================================

testar_validacoes_unitarias() {
    print_section "FASE 1: Testes Unitários de Validação"
    
    if [ -f "./test_validacoes" ]; then
        ./test_validacoes
    else
        print_warning "test_validacoes não encontrado, pulando..."
    fi
}

testar_queries() {
    print_section "FASE 2: Testes de Queries (outputs .txt)"
    
    if ! check_dir "$DATASET_NORMAL"; then
        print_error "Dataset normal não encontrado"
        return 1
    fi
    
    if ! check_file "$INPUT_FILE"; then
        print_error "Ficheiro de inputs não encontrado"
        return 1
    fi
    
    print_warning "Gerando resultados..."
    $PROGRAMA_PRINCIPAL "$DATASET_NORMAL" "$INPUT_FILE" > /dev/null 2>&1
    
    if [ -f "./teste" ] && [ -d "$ESPERADOS" ]; then
        print_warning "Comparando com esperados..."
        ./teste "$DATASET_NORMAL" "$INPUT_FILE" "$ESPERADOS"
    else
        print_warning "Ficheiros esperados não disponíveis para comparação"
    fi
}

testar_falsos_positivos() {
    print_section "FASE 3: Análise de Falsos Positivos (Dataset Limpo)"
    
    if ! check_dir "$DATASET_LIMPO"; then
        print_warning "Dataset limpo não encontrado em '$DATASET_LIMPO'"
        print_warning "Pulando análise de falsos positivos..."
        return 1
    fi
    
    print_warning "Gerando resultados com dataset limpo..."
    $PROGRAMA_PRINCIPAL "$DATASET_LIMPO" "$INPUT_FILE" > /dev/null 2>&1
    
    if [ -f "./teste_csv" ]; then
        ./teste_csv falsos-positivos "$DATASET_LIMPO" "$RESULTADOS/Erros"
    else
        print_error "teste_csv não encontrado"
    fi
}

testar_falsos_negativos() {
    print_section "FASE 4: Análise de Falsos Negativos (Dataset com Erros)"
    
    if ! check_dir "$DATASET_ERROS"; then
        print_warning "Dataset com erros não encontrado em '$DATASET_ERROS'"
        print_warning "Pulando análise de falsos negativos..."
        return 1
    fi
    
    print_warning "Gerando resultados com dataset de erros..."
    $PROGRAMA_PRINCIPAL "$DATASET_ERROS" "$INPUT_FILE" > /dev/null 2>&1
    
    if [ -f "./teste_csv" ]; then
        ./teste_csv falsos-negativos "$DATASET_ERROS" "$RESULTADOS/Erros"
    else
        print_error "teste_csv não encontrado"
    fi
}

comparar_csvs() {
    print_section "FASE 5: Comparação com CSVs Esperados"
    
    if ! check_dir "$ESPERADOS/Erros"; then
        print_warning "CSVs esperados não disponíveis"
        print_warning "Disponíveis após às 12h na plataforma"
        return 1
    fi
    
    if [ -f "./teste_csv" ]; then
        ./teste_csv comparar "$RESULTADOS/Erros" "$ESPERADOS/Erros"
    else
        print_error "teste_csv não encontrado"
    fi
}

# ============================================================================
# WORKFLOWS
# ============================================================================

workflow_completo() {
    print_header "WORKFLOW COMPLETO DE TESTES"
    
    testar_validacoes_unitarias
    testar_queries
    testar_falsos_positivos
    testar_falsos_negativos
    comparar_csvs
    
    print_section "RESUMO FINAL"
    mostrar_estatisticas
}

workflow_basico() {
    print_header "WORKFLOW BÁSICO (Sem datasets especiais)"
    
    testar_validacoes_unitarias
    testar_queries
    
    print_section "RESUMO"
    print_warning "Para testes completos, forneça:"
    echo "  - $DATASET_LIMPO/ (dataset sem erros)"
    echo "  - $DATASET_ERROS/ (dataset com erros conhecidos)"
    echo "  - $ESPERADOS/ (resultados esperados da plataforma)"
}

workflow_debug() {
    print_header "MODO DEBUG (Análise Detalhada)"
    
    print_section "Verificando estrutura de ficheiros"
    echo "Estrutura esperada:"
    echo "  ✓ Programa: $PROGRAMA_PRINCIPAL"
    check_file "$PROGRAMA_PRINCIPAL" && print_success "Encontrado" || print_error "Não encontrado"
    
    echo "  Inputs: $INPUT_FILE"
    check_file "$INPUT_FILE" && print_success "Encontrado" || print_error "Não encontrado"
    
    echo "   Dataset normal: $DATASET_NORMAL/"
    check_dir "$DATASET_NORMAL" && print_success "Encontrado" || print_error "Não encontrado"
    
    echo "   Dataset limpo: $DATASET_LIMPO/"
    check_dir "$DATASET_LIMPO" && print_success "Encontrado" || print_warning "Não encontrado (opcional)"
    
    echo "   Dataset erros: $DATASET_ERROS/"
    check_dir "$DATASET_ERROS" && print_success "Encontrado" || print_warning "Não encontrado (opcional)"
    
    echo "   Esperados: $ESPERADOS/"
    check_dir "$ESPERADOS" && print_success "Encontrado" || print_warning "Não encontrado (disponível às 12h)"
    
    print_section "Conteúdo dos datasets"
    for dataset in "$DATASET_NORMAL" "$DATASET_LIMPO" "$DATASET_ERROS"; do
        if [ -d "$dataset" ]; then
            echo ""
            echo "$dataset/:"
            ls -lh "$dataset"/*.csv 2>/dev/null | awk '{print "  "$9" ("$5")"}'
        fi
    done
    
    print_section "Estatísticas de resultados anteriores"
    mostrar_estatisticas
}

mostrar_estatisticas() {
    if [ ! -d "$RESULTADOS" ]; then
        print_warning "Nenhum resultado gerado ainda"
        return
    fi
    
    echo ""
    echo "Queries (.txt):"
    local txt_count=$(ls "$RESULTADOS"/*.txt 2>/dev/null | wc -l)
    echo "  Total: $txt_count ficheiros"
    
    if [ -d "$RESULTADOS/Erros" ]; then
        echo ""
        echo "CSVs de Erros:"
        for csv in "$RESULTADOS/Erros"/*.csv; do
            if [ -f "$csv" ]; then
                local linhas=$(wc -l < "$csv")
                local nome=$(basename "$csv")
                echo "  $nome: $linhas linhas"
            fi
        done
    fi
}

# ============================================================================
# AÇÕES RÁPIDAS
# ============================================================================

quick_check() {
    print_header "VERIFICAÇÃO RÁPIDA"
    
    # Compila se necessário
    if [ ! -f "./teste" ] || [ ! -f "./teste_csv" ]; then
        compilar
    fi
    
    # Testa só o essencial
    if [ -f "./test_validacoes" ]; then
        print_section "Validações Unitárias"
        ./test_validacoes | tail -n 20
    fi
    
    print_section "Estatísticas Rápidas"
    mostrar_estatisticas
}

limpar() {
    print_section "Limpando ficheiros gerados"
    
    read -p "Remover resultados? (s/n): " resposta
    if [ "$resposta" = "s" ]; then
        rm -rf "$RESULTADOS"
        print_success "Resultados removidos"
    fi
    
    read -p "Remover executáveis de teste? (s/n): " resposta
    if [ "$resposta" = "s" ]; then
        rm -f teste teste_csv test_validacoes
        print_success "Executáveis removidos"
    fi
}

# ============================================================================
# INTERFACE
# ============================================================================

mostrar_ajuda() {
    cat << EOF

$(tput bold)Sistema de Testes Automatizado - LI3$(tput sgr0)
Replica o processo de validação da plataforma

$(tput bold)USO:$(tput sgr0)
  $0 <comando> [opções]

$(tput bold)COMANDOS:$(tput sgr0)

  $(tput bold)compilar$(tput sgr0)         Compila todos os programas de teste
  
  $(tput bold)completo$(tput sgr0)         Executa workflow completo (todas as fases)
  $(tput bold)basico$(tput sgr0)           Workflow básico (validações + queries)
  $(tput bold)debug$(tput sgr0)            Modo debug com análise detalhada
  $(tput bold)quick$(tput sgr0)            Verificação rápida
  
  $(tput bold)validacoes$(tput sgr0)       Executa apenas testes unitários
  $(tput bold)queries$(tput sgr0)          Testa apenas outputs .txt
  $(tput bold)falsos-pos$(tput sgr0)       Detecta falsos positivos
  $(tput bold)falsos-neg$(tput sgr0)       Detecta falsos negativos
  $(tput bold)comparar$(tput sgr0)         Compara CSVs com esperados
  
  $(tput bold)stats$(tput sgr0)            Mostra estatísticas
  $(tput bold)limpar$(tput sgr0)           Remove ficheiros gerados
  $(tput bold)help$(tput sgr0)             Mostra esta ajuda

$(tput bold)EXEMPLOS:$(tput sgr0)

  # Primeira vez (compilar + testar tudo)
  $0 compilar && $0 completo
  
  # Durante desenvolvimento (verificação rápida)
  $0 quick
  
  # Testar apenas queries
  $0 queries
  
  # Debug completo
  $0 debug

$(tput bold)ESTRUTURA NECESSÁRIA:$(tput sgr0)

  projeto/
  ├── programa-principal    (teu programa)
  ├── inputs               (queries)
  ├── dataset/             (dataset normal)
  ├── dataset_limpo/       (opcional: sem erros)
  ├── dataset_erros/       (opcional: com erros)
  └── resultados_esperados/ (da plataforma)

EOF
}

# ============================================================================
# MAIN
# ============================================================================

main() {
    case "${1:-help}" in
        compilar)
            compilar
            ;;
        completo)
            workflow_completo
            ;;
        basico)
            workflow_basico
            ;;
        debug)
            workflow_debug
            ;;
        quick)
            quick_check
            ;;
        validacoes)
            testar_validacoes_unitarias
            ;;
        queries)
            testar_queries
            ;;
        falsos-pos)
            testar_falsos_positivos
            ;;
        falsos-neg)
            testar_falsos_negativos
            ;;
        comparar)
            comparar_csvs
            ;;
        stats)
            mostrar_estatisticas
            ;;
        limpar)
            limpar
            ;;
        help|--help|-h)
            mostrar_ajuda
            ;;
        *)
            print_error "Comando desconhecido: $1"
            echo ""
            mostrar_ajuda
            exit 1
            ;;
    esac
}

# Executa
main "$@"