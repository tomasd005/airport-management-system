#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "gestor_programa.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Uso: %s <pasta_dados> <ficheiro_input>\n", argv[0]);
        return 1;
    }

    const char *pastaDados = argv[1];
    const char *ficheiroInput = argv[2];

    // Criar pasta resultados se não existir
    mkdir("resultados", 0755);

    // Criar gestor do programa (modo normal, sem economia de memória para performance)
    GestorDePrograma *gestor = gestor_programa_novo(FALSE);
    if (!gestor)
    {
        fprintf(stderr, "Erro ao criar gestor\n");
        return 1;
    }

    // Executar
    gestor_programa_executa(gestor, pastaDados, ficheiroInput);

    // Libertar memória
    gestor_programa_destroi(gestor);

    return 0;
}