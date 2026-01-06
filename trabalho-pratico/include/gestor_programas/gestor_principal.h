#ifndef GESTOR_PRINCIPAL_H
#define GESTOR_PRINCIPAL_H

#include "gestores/gestor_aeroportos.h"
#include "gestores/gestor_avioes.h"
#include "gestores/gestor_voos.h"
#include "gestores/gestor_passageiros.h"
#include "gestores/gestor_reservas.h"

/**
 * @brief Estrutura opaca para o gestor do programa principal
 */
typedef struct GestorPrincipal gestor_principal_t;

/**
 * @brief Cria um novo gestor do programa principal
 * @return Ponteiro para o gestor criado
 */
gestor_principal_t *gestor_principal_criar(void);

/**
 * @brief Executa o programa principal (batch mode)
 * @param gestor Gestor do programa
 * @param pasta_dados Caminho para a pasta com os CSVs
 * @param ficheiro_input Caminho para o ficheiro de input com queries
 */
void gestor_principal_executar(
    gestor_principal_t *gestor,
    const char *pasta_dados,
    const char *ficheiro_input);

/**
 * @brief Destrói o gestor e liberta recursos
 * @param gestor Gestor a destruir
 */
void gestor_principal_destruir(gestor_principal_t *gestor);

#endif // GESTOR_PRINCIPAL_H