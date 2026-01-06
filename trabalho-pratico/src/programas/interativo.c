#include "gestor_interativo.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Programa em modo INTERATIVO
 *
 * Permite ao utilizador executar queries interativamente
 * através de um menu.
 *
 * Uso: ./programa-interativo
 */
int main(void)
{
    // Criar gestor interativo
    gestor_interativo_t *gestor = gestor_interativo_criar();
    if (!gestor)
    {
        fprintf(stderr, "Erro ao criar gestor interativo\n");
        return 1;
    }

    // Executar modo interativo
    gestor_interativo_executar(gestor);

    // Limpar recursos
    gestor_interativo_destruir(gestor);

    return 0;
}