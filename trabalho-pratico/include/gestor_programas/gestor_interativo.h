#ifndef GESTOR_INTERATIVO_H
#define GESTOR_INTERATIVO_H

/**
 * @brief Estrutura opaca para o gestor interativo
 */
typedef struct GestorInterativo gestor_interativo_t;

/**
 * @brief Cria um novo gestor interativo
 * @return Ponteiro para o gestor criado
 */
gestor_interativo_t *gestor_interativo_criar(void);

/**
 * @brief Executa o programa em modo interativo
 * @param gestor Gestor interativo
 */
void gestor_interativo_executar(gestor_interativo_t *gestor);

/**
 * @brief Destrói o gestor e liberta recursos
 * @param gestor Gestor a destruir
 */
void gestor_interativo_destruir(gestor_interativo_t *gestor);

#endif // GESTOR_INTERATIVO_H