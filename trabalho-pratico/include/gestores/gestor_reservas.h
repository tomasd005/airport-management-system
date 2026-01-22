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
 *
 * @return Ponteiro para o gestor criado.
 */
gestor_reservas_t *gestor_reservas_criar(void);

/**
 * @brief Destroi o gestor de reservas e liberta recursos.
 *
 * @param gestor Gestor de reservas.
 */
void gestor_reservas_destruir(gestor_reservas_t *gestor);

/**
 * @brief Retorna o número de reservas carregadas.
 *
 * @param gestor Gestor de reservas.
 * @return Número de reservas.
 */
unsigned int gestor_reservas_numero(gestor_reservas_t *gestor);

/**
 * @brief Carrega reservas de um CSV com validação e agregações.
 *
 * @param gestor Gestor de reservas.
 * @param ficheiro_csv Caminho do CSV.
 * @param gestor_voos Gestor de voos para validação.
 * @param gestor_passageiros Gestor de passageiros para validação.
 */
void gestor_reservas_carregar_com_validacao(gestor_reservas_t *gestor, const char *ficheiro_csv, gestor_voos_t *gestor_voos, gestor_passageiros_t *gestor_passageiros);

/**
 * @brief Finaliza agregações de reservas (top10, destinos).
 *
 * @param gestor Gestor de reservas.
 */
void gestor_reservas_finalizar(gestor_reservas_t *gestor);

/**
 * @brief Obtém o top10 de passageiros por semana.
 *
 * @param gestor Gestor de reservas.
 * @param semana Identificador da semana.
 * @return Array com até 10 passageiros.
 */
const GPtrArray *gestor_reservas_obter_top10_semana(gestor_reservas_t *gestor, int semana);

/**
 * @brief Obtém o destino mais comum para uma nacionalidade.
 *
 * @param gestor Gestor de reservas.
 * @param nac Nacionalidade.
 * @param destino Output do código do destino.
 * @param count Output da contagem.
 * @return TRUE se existir destino, FALSE caso contrário.
 */
gboolean gestor_reservas_obter_melhor_destino_nacionalidade(gestor_reservas_t *gestor, const char *nac, const char **destino, guint *count);

/**
 * @brief Aplica um callback a cada top10 semanal.
 *
 * @param gestor Gestor de reservas.
 * @param callback Função a executar.
 * @param user_data Dados do utilizador.
 */
void gestor_reservas_para_cada_top10(gestor_reservas_t *gestor, void (*callback)(int semana, const GPtrArray *top10, void *user_data), void *user_data);

#endif
