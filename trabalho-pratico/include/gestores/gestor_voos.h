#ifndef GESTOR_VOOS_H
#define GESTOR_VOOS_H

#include "../entidades/voos.h"
#include <glib.h>
#include <stdint.h>

/**
 * @file gestor_voos.h
 * @brief Gestão centralizada de voos.
 *
 * Este módulo permite criar, destruir e gerir um conjunto de voos,
 * incluindo inserção, carregamento com ou sem validação, consultas
 * estatísticas e atualização de contagens relacionadas a aeroportos e companhias.
 */

typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_avioes gestor_avioes_t;
typedef struct gestor_aeroportos gestor_aeroportos_t;

/**
 * @brief Estrutura para armazenar estatísticas de uma companhia aérea (para query 5).
 */
typedef struct
{
    char *airline;
    guint count;
    double avg_delay;
} gestor_voos_q5_t;

/**
 * @brief Cria um gestor de voos.
 * @return Gestor alocado, ou NULL em erro de memória.
 */
gestor_voos_t *gestor_voos_criar(void);

/**
 * @brief Destrói o gestor e liberta todos os voos/índices.
 * @param gestor Gestor a destruir (aceita NULL).
 */
void gestor_voos_destruir(gestor_voos_t *gestor);

/**
 * @brief Adiciona um voo a partir de info parseada.
 * @param gestor Gestor de voos.
 * @param info Informação do voo (ownership transferido).
 */
void gestor_voos_adicionar(gestor_voos_t *gestor, voo_info_t *info);

/**
 * @brief Obtém voo por flight_id.
 * @param gestor Gestor de voos.
 * @param flight_id Identificador do voo.
 * @return Ponteiro para o voo, ou NULL se não existir.
 */
voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id);

/**
 * @brief Obtém voo por chave compacta.
 * @param gestor Gestor de voos.
 * @param key Chave compacta.
 * @return Ponteiro para o voo, ou NULL se não existir.
 */
voo_t *gestor_voos_obter_por_key(gestor_voos_t *gestor, uint64_t key);

/**
 * @brief Devolve o número de voos válidos.
 * @param gestor Gestor de voos.
 * @return Total de voos.
 */
unsigned int gestor_voos_contar(const gestor_voos_t *gestor);

/**
 * @brief Carrega voos a partir de CSV (sem validação lógica extra).
 * @param gestor Gestor de voos.
 * @param ficheiro_csv Caminho para o CSV.
 */
void gestor_voos_carregar(gestor_voos_t *gestor, const char *ficheiro_csv);

/**
 * @brief Carrega voos com validação lógica adicional.
 * @param gestor Gestor de voos.
 * @param ficheiro_csv Caminho para o CSV.
 * @param gestor_avioes Gestor de aviões (validação).
 */
void gestor_voos_carregar_com_validacao(gestor_voos_t *gestor, const char *ficheiro_csv, gestor_avioes_t *gestor_avioes);

/**
 * @brief Adiciona voo já validado, aplicando validação lógica extra (aeronave).
 *
 * @param gestor Gestor de voos
 * @param gestor_avioes Gestor de aviões
 * @param info Estrutura de voo já parseada/validada
 * @return TRUE se adicionado com sucesso
 * @note Ownership: em caso de sucesso, o gestor passa a ser dono de info.
 */
gboolean gestor_voos_adicionar_validado(gestor_voos_t *gestor, gestor_avioes_t *gestor_avioes,
                                        voo_info_t *info);

/**
 * @brief Itera por cada voo e invoca um callback.
 * @param gestor Gestor de voos.
 * @param callback Função chamada para cada voo.
 * @param user_data Contexto do utilizador.
 */
void gestor_voos_para_cada(gestor_voos_t *gestor, void (*callback)(voo_t *, void *), void *user_data);

/**
 * @brief Prepara estruturas auxiliares da query 3.
 * @param gestor Gestor de voos.
 */
void gestor_voos_preparar_q3(gestor_voos_t *gestor);

/**
 * @brief Obtém a origem com mais partidas num intervalo de dias.
 * @param gestor Gestor de voos.
 * @param dia_inicio Dia inicial (índice).
 * @param dia_fim Dia final (índice).
 * @param out_origem Saída com o código de origem.
 * @param out_contagem Saída com a contagem.
 * @return TRUE se existir resultado, FALSE caso contrário.
 */
gboolean gestor_voos_melhor_origem_intervalo(gestor_voos_t *gestor, int dia_inicio, int dia_fim, const char **out_origem, guint *out_contagem);

/**
 * @brief Itera sobre contagens de atraso por companhia.
 * @param gestor Gestor de voos.
 * @param callback Função chamada para cada companhia.
 * @param user_data Contexto do utilizador.
 */
void gestor_voos_para_cada_atraso(gestor_voos_t *gestor, void (*callback)(const char *airline, guint count, double total_delay, void *), void *user_data);

/**
 * @brief Obtém a cache ordenada da query 5.
 * @param gestor Gestor de voos.
 * @return Array com estatísticas por companhia.
 */
const GArray *gestor_voos_obter_q5_cache(gestor_voos_t *gestor);

/**
 * @brief Regista passageiros num voo e nos agregados por aeroporto.
 *
 * Mantém a atualização das contagens dentro do gestor de voos, evitando que
 * outros módulos conheçam o layout interno de voo_t ou dos agregados.
 *
 * @param gestor Gestor de voos.
 * @param voo Voo a atualizar.
 * @param delta Número de passageiros a adicionar.
 */
void gestor_voos_registar_passageiros(gestor_voos_t *gestor, voo_t *voo, int delta);

/**
 * @brief Atualiza contagens de partidas/chegadas nos aeroportos.
 * @param gestor_voos Gestor de voos.
 * @param gestor_aeroportos Gestor de aeroportos.
 */
void gestor_voos_atualizar_contagens_aeroportos(gestor_voos_t *gestor_voos, gestor_aeroportos_t *gestor_aeroportos);

/**
 * @brief Descarta a tabela detalhada de voos após pré-processamento.
 *
 * Mantém apenas índices/agregados usados nas queries (Q3 e Q5),
 * reduzindo memória no modo de execução normal.
 *
 * @param gestor Gestor de voos.
 */
void gestor_voos_descartar_tabela(gestor_voos_t *gestor);

#endif
