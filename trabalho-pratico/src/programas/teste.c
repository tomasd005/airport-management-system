#include "gestor_teste.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Programa de TESTES automáticos
 *
 * Executa o programa principal e compara os resultados
 * com os outputs esperados.
 *
 * Uso: ./programa-testes <pasta_datasets> <ficheiro_input> <pasta_esperados>
 */
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Uso: %s <pasta_datasets> <ficheiro_input> <pasta_esperados>\n", argv[0]);
        return 1;
    }

    const char *pasta_dataset = argv[1];
    const char *ficheiro_input = argv[2];
    const char *pasta_esperados = argv[3];

    // Criar gestor de testes
    gestor_testes_t *gestor = gestor_testes_criar();
    if (!gestor)
    {
        fprintf(stderr, "Erro ao criar gestor de testes\n");
        return 1;
    }

    // Executar testes
    int resultado = gestor_testes_executar(
        gestor,
        pasta_dataset,
        ficheiro_input,
        pasta_esperados);

    // Limpar recursos
    gestor_testes_destruir(gestor);

    return resultado;
}