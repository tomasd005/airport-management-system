#!/bin/bash
# Mostra outputs esperados vs obtidos lado a lado

if [ "$#" -lt 1 ]; then
    echo "Uso: $0 <numero_query>"
    echo "Exemplo: $0 151"
    exit 1
fi

Q=$1
ESPERADO="resultados-esperados/command${Q}_output.txt"
OBTIDO="resultados/command${Q}_output.txt"

echo "=========================================="
echo "QUERY $Q"
echo "=========================================="
echo ""

COMANDO=$(sed -n "${Q}p" inputs_fase2.txt 2>/dev/null)
echo "Comando: $COMANDO"
echo ""

echo "ESPERADO                                          | OBTIDO"
echo "--------------------------------------------------|--------------------------------------------------"

if [ ! -f "$ESPERADO" ] || [ ! -f "$OBTIDO" ]; then
    echo "❌ Um dos ficheiros não existe"
    exit 1
fi

# Usar paste para mostrar lado a lado
paste <(cat "$ESPERADO") <(cat "$OBTIDO") | awk -F'\t' '{printf "%-50s | %s\n", $1, $2}'

echo ""
echo "Linhas: Esperado=$(wc -l < "$ESPERADO")  Obtido=$(wc -l < "$OBTIDO")"