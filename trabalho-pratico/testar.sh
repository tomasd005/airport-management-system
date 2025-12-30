#!/bin/bash

echo "═══════════════════════════════════════════════════"
echo "DIAGNÓSTICO URGENTE: Q5 voltou a 0/50!"
echo "═══════════════════════════════════════════════════"
echo

echo "1. Verificar output Q5:"
echo "───────────────────────────────────────────────────"
echo "Esperado:"
head -3 resultados-esperados/command201_output.txt
echo
echo "Seu (atual):"
head -3 resultados/command201_output.txt
echo
echo "Tamanho do ficheiro seu:"
ls -lh resultados/command201_output.txt
echo

echo "2. Verificar parser (deve aceitar barras):"
echo "───────────────────────────────────────────────────"
grep -A5 "sscanf.*datetime" src/parsers/parser.c | head -15
echo

echo "3. Teste rápido do parser:"
echo "───────────────────────────────────────────────────"
echo "Criar teste..."

cat > /tmp/test_parser.c << 'TESTEOF'
#include <stdio.h>
#include <time.h>

// Copiar a função parser_datetime_para_time aqui
time_t parser_datetime_para_time(const char *datetime);

int main() {
    time_t t1 = parser_datetime_para_time("2025-01-20 19:30");
    time_t t2 = parser_datetime_para_time("2025/01/20 19:30");
    
    printf("Com hífens: %ld %s\n", (long)t1, t1 > 0 ? "OK" : "FALHOU");
    printf("Com barras: %ld %s\n", (long)t2, t2 > 0 ? "OK" : "FALHOU");
    
    if (t1 > 0 && t2 > 0) {
        printf("✅ Parser aceita AMBOS os formatos\n");
    } else if (t1 > 0 && t2 < 0) {
        printf("❌ Parser só aceita hífens\n");
    } else if (t1 < 0 && t2 > 0) {
        printf("❌ Parser só aceita barras\n");
    } else {
        printf("❌ Parser não funciona!\n");
    }
    
    return 0;
}
TESTEOF

echo "Ficheiro de teste criado em /tmp/test_parser.c"
echo

echo "4. Verificar voos Delayed carregados:"
echo "───────────────────────────────────────────────────"
echo "Adicione temporariamente debug em gestor_voos.c:"
echo
cat << 'DEBUGCODE'
// Em gestor_voos_adicionar(), após adicionar ao array atrasados:
if (voo_obter_status(voo) && strcmp(voo_obter_status(voo), "Delayed") == 0) {
    g_ptr_array_add(gestor->atrasados, voo);
    
    // DEBUG temporário
    static int count = 0;
    if (count < 3) {
        fprintf(stderr, "DEBUG: Voo Delayed adicionado: %s, airline=%s, delay=%.2f\n",
                voo_obter_id(voo), 
                voo_obter_airline(voo),
                voo_calcular_atraso_minutos(voo));
        count++;
    }
}
DEBUGCODE
echo

echo "5. Executar com debug:"
echo "───────────────────────────────────────────────────"
echo "./programa-principal com_erros/ inputs_fase2.txt 2>&1 | grep DEBUG | head -10"
echo

echo "═══════════════════════════════════════════════════"
echo "CHECKLIST DE VERIFICAÇÃO:"
echo "═══════════════════════════════════════════════════"
echo
echo "[ ] Output Q5 está vazio (só \\n)?"
echo "[ ] Parser aceita formato com barras?"
echo "[ ] Voos Delayed estão sendo carregados?"
echo "[ ] delay_minutes está sendo calculado?"
echo "[ ] gestor->atrasados tem elementos?"
echo

echo "═══════════════════════════════════════════════════"

