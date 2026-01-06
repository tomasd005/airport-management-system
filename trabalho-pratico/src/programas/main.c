#include <stdio.h>
#include <string.h>
#include "gestor_programa/gestor_programa.h"

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        fprintf(stderr, "Uso: %s <pasta_dados> <ficheiro_input> [normal]\n", argv[0]);
        return 1;
    }

    const char *pastaDados = argv[1];
    const char *ficheiroInput = argv[2];

    gboolean modoEconomiaMemoria = TRUE; // default: modo economia de memória

    if (argc > 3 && strcmp(argv[3], "normal") == 0)
    {
        modoEconomiaMemoria = FALSE;
    }

    printf("Programa principal inicializado no modo %s\n",
           modoEconomiaMemoria ? "Economia de memória" : "Normal");

    // Criar gestor do programa
    GestorDePrograma *gestor = gestor_programa_novo(modoEconomiaMemoria);

    // Executar
    gestor_programa_executa(gestor, pastaDados, ficheiroInput);

    // Libertar memória
    gestor_programa_destroi(gestor);

    printf("Execução concluída com sucesso.\n");
    return 0;
}
