#ifndef GESTOR_RESERVAS_H
#define GESTOR_RESERVAS_H

#include <glib.h>

/**
 * @file gestor_reservas.h
 * @brief Gestão centralizada de reservas de voos.
 *
 * Este módulo permite criar, destruir e gerir um conjunto de reservas,
 * incluindo inserção com validação, consultas e estatísticas como top10 ou
 * destinos mais populares por nacionalidade.
 */

typedef struct gestor_reservas gestor_reservas_t;
typedef struct gestor_voos gestor_voos_t;
typedef struct gestor_passageiros gestor_passageiros_t;

/**
 * @brief Cria um gestor de reservas.
 * @return Gestor alocado, ou NULL em erro de memória.
 */
gestor_reservas_t *gestor_reservas_criar(void);

/**
 * @brief Destrói o gestor e liberta todas as reservas/índices.
 * @param gestor Gestor a destruir (aceita NULL).
 */
void gestor_reservas_destruir(gestor_reservas_t *gestor);

/**
 * @brief Devolve o número de reservas válidas.
 * @param gestor Gestor de reservas.
 * @return Total de reservas.
 */
unsigned int gestor_reservas_numero(gestor_reservas_t *gestor);

/**
 * @brief Carrega reservas com validação sintática e lógica.
 * @param gestor Gestor de reservas.
 * @param ficheiro_csv Caminho para o CSV.
 * @param gestor_voos Gestor de voos (validação lógica).
 * @param gestor_passageiros Gestor de passageiros (validação lógica).
 */
void gestor_reservas_carregar_com_validacao(gestor_reservas_t *gestor, const char *ficheiro_csv, gestor_voos_t *gestor_voos, gestor_passageiros_t *gestor_passageiros);

/**
 * @brief Finaliza índices agregados após carregamento.
 * @param gestor Gestor de reservas.
 */
void gestor_reservas_finalizar(gestor_reservas_t *gestor);

/**
 * @brief Obtém o top10 semanal de passageiros por gasto.
 * @param gestor Gestor de reservas.
 * @param semana Semana (inteiro).
 * @return Ponteiro para array de top10, ou NULL se inexistente.
 */
const GPtrArray *gestor_reservas_obter_top10_semana(gestor_reservas_t *gestor, int semana);

/**
 * @brief Obtém o destino mais comum para uma nacionalidade.
 * @param gestor Gestor de reservas.
 * @param nac Nacionalidade.
 * @param destino Saída com o destino.
 * @param count Saída com o número de ocorrências.
 * @return TRUE se existir resultado, FALSE caso contrário.
 */
gboolean gestor_reservas_obter_melhor_destino_nacionalidade(gestor_reservas_t *gestor, const char *nac, const char **destino, guint *count);

/**
 * @brief Itera sobre todos os top10 semanais.
 * @param gestor Gestor de reservas.
 * @param callback Função chamada por semana.
 * @param user_data Contexto do utilizador.
 */
void gestor_reservas_para_cada_top10(gestor_reservas_t *gestor, void (*callback)(int semana, const GPtrArray *top10, void *user_data), void *user_data);

/**
 * @brief Coleta chaves de documentos presentes nos top10.
 * @param gestor Gestor de reservas.
 * @param doc_keys HashTable de chaves (out).
 */
void gestor_reservas_coletar_docs_top10(gestor_reservas_t *gestor, GHashTable *doc_keys);

#endif
