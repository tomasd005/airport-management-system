#!/bin/bash

echo "=== Verificação de Ficheiros CSV de Erros ==="
echo ""

EXPECTED_DIR="com_erros"
RESULTS_DIR="resultados"

EXPECTED_FILES=(
    "flights_errors.csv"
    "airports_errors.csv"
    "aircrafts_errors.csv"
    "passengers_errors.csv"
    "reservations_errors.csv"
)

echo "📁 Diretório esperado: $EXPECTED_DIR"
echo "📁 Diretório resultados: $RESULTS_DIR"
echo ""
echo "🔍 Verificando ficheiros de erros..."
echo ""

MISSING=0
FOUND=0

for file in "${EXPECTED_FILES[@]}"; do
    EXPECTED_PATH="$EXPECTED_DIR/$file"
    RESULT_PATH="$RESULTS_DIR/$file"
    
    if [ ! -f "$EXPECTED_PATH" ]; then
        echo "⚠️  $file: Ficheiro esperado NÃO existe em $EXPECTED_DIR"
        continue
    fi
    
    if [ ! -f "$RESULT_PATH" ]; then
        echo "❌ $file: FALTA em $RESULTS_DIR"
        ((MISSING++))
    else
        echo "✅ $file: Encontrado"
        ((FOUND++))
        
        EXPECTED_LINES=$(wc -l < "$EXPECTED_PATH")
        RESULT_LINES=$(wc -l < "$RESULT_PATH")
        
        echo "   └─ Esperado: $EXPECTED_LINES linhas | Gerado: $RESULT_LINES linhas"
        
        DIFF=$((RESULT_LINES - EXPECTED_LINES))
        if [ $DIFF -gt 0 ]; then
            echo "   └─ ⚠️  Tem $DIFF linhas A MAIS"
        elif [ $DIFF -lt 0 ]; then
            echo "   └─ ⚠️  Tem ${DIFF#-} linhas A MENOS"
        else
            echo "   └─ ✓ Número de linhas correto"
        fi
    fi
    echo ""
done

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 Resumo:"
echo "   ✅ Encontrados: $FOUND"
echo "   ❌ Em falta: $MISSING"
echo ""
