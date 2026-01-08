#ifndef GESTOR_TESTES_H
#define GESTOR_TESTES_H

/**
 * @brief Estrutura opaca para o gestor de testes
 */
typedef struct GestorTestes gestor_testes_t;

/**
 * @brief Cria um novo gestor de testes
 * @return Ponteiro para o gestor criado
 */
gestor_testes_t *gestor_testes_criar(void);

/**
 * @brief Executa o programa de testes
 * @param gestor Gestor de testes
 * @param pasta_dataset Caminho para a pasta com os CSVs
 * @param ficheiro_input Caminho para o ficheiro de input com queries
 * @param pasta_esperados Caminho para a pasta com resultados esperados
 * @return 0 se todos os testes passarem, 1 caso contrário
 */
int gestor_testes_executar(
    gestor_testes_t *gestor,
    const char *pasta_dataset,
    const char *ficheiro_input,
    const char *pasta_esperados);

/**
 * @brief Destrói o gestor e liberta recursos
 * @param gestor Gestor a destruir
 */
void gestor_testes_destruir(gestor_testes_t *gestor);

#endif // GESTOR_TESTES_H