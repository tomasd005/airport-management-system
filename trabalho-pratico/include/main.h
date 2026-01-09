#ifndef MAIN_H
#define MAIN_H

/**
 * @file main.h
 * @brief Declarações das funções principais do programa.
 *
 * Contém as entradas principais para execução normal e para execução
 * de testes automatizados.
 */

/**
 * @brief Função principal do programa para execução normal.
 *
 * Inicializa o gestor do programa, processa arquivos de dados e
 * executa o fluxo principal da aplicação.
 *
 * @param argc Número de argumentos da linha de comando.
 * @param argv Array de strings com os argumentos da linha de comando.
 * @return Código de saída (0 em sucesso, diferente de 0 em erro).
 */
int main_principal(int argc, char *argv[]);

/**
 * @brief Função principal para execução de testes.
 *
 * Inicializa o programa em modo de testes automatizados, carregando
 * dados de teste e executando funções específicas para verificação.
 *
 * @param argc Número de argumentos da linha de comando.
 * @param argv Array de strings com os argumentos da linha de comando.
 * @return Código de saída (0 em sucesso, diferente de 0 em erro).
 */
int main_testes(int argc, char *argv[]);

#endif
