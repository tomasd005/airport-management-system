#!/bin/bash

# Script para debugar queries vazias

echo "=== VERIFICANDO DADOS CARREGADOS ==="
echo ""
echo "Adicione estes prints TEMPORÁRIOS no main ou gestor_principal:"
echo ""
cat << 'EOF'
// No final do carregamento de dados, adicionar:
fprintf(stderr, "\n=== VERIFICAÇÃO DE DADOS ===\n");
fprintf(stderr, "Aeroportos: %u\n", gestor_aeroportos_contar(gestor->aeroportos));
fprintf(stderr, "Voos: %u\n", gestor_voos_contar(gestor->voos));
fprintf(stderr, "Passageiros: %u\n", gestor_passageiros_numero(gestor->passageiros));
fprintf(stderr, "Reservas: %u\n", gestor_reservas_numero(gestor->reservas));

// Testar um aeroporto específico da Q1
aeroporto_t *teste = gestor_aeroportos_obter_por_codigo(gestor->aeroportos, "SWU");
fprintf(stderr, "Teste aeroporto SWU: %p\n", teste);

// Testar voos por origem
GPtrArray *voos_origem = gestor_voos_obter_por_origin(gestor->voos, "SWU");
fprintf(stderr, "Voos com origem SWU: %u\n", voos_origem ? voos_origem->len : 0);

// Testar voos por destino
GPtrArray *voos_dest = gestor_voos_obter_por_destination(gestor->voos, "SWU");
fprintf(stderr, "Voos com destino SWU: %u\n", voos_dest ? voos_dest->len : 0);

// Testar contagem de passageiros em um voo
if (voos_origem && voos_origem->len > 0) {
    voo_t *voo_teste = g_ptr_array_index(voos_origem, 0);
    const char *flight_id = voo_obter_id(voo_teste);
    int pass_count = gestor_reservas_contar_passageiros_voo(gestor->reservas, flight_id);
    fprintf(stderr, "Voo %s tem %d passageiros\n", flight_id, pass_count);
}

// Testar passageiros por nacionalidade para Q6
GPtrArray *pass_pt = gestor_passageiros_obter_por_nacionalidade(gestor->passageiros, "PT");
fprintf(stderr, "Passageiros portugueses: %u\n", pass_pt ? pass_pt->len : 0);

fprintf(stderr, "=========================\n\n");
EOF

echo ""
echo "Compile e execute para ver o que está a falhar:"
echo "make clean && make && ./programa-testes sem_erros/ inputs_fase2.txt 2>&1 | grep -A 20 'VERIFICAÇÃO'"