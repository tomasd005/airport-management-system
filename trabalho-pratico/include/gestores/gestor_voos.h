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
 *
 * @return Ponteiro para o gestor criado.
 */
gestor_voos_t *gestor_voos_criar(void);

/**
 * @brief Destroi o gestor de voos e liberta recursos.
 *
 * @param gestor Gestor de voos.
 */
void gestor_voos_destruir(gestor_voos_t *gestor);

/**
 * @brief Adiciona um voo ao gestor.
 *
 * @param gestor Gestor de voos.
 * @param voo Voo a adicionar.
 */
void gestor_voos_adicionar(gestor_voos_t *gestor, voo_t *voo);

/**
 * @brief Regista um voo que recebeu passageiros (otimização Q1).
 *
 * @param gestor Gestor de voos.
 * @param voo Voo a registar.
 */
void gestor_voos_registar_voo_com_passageiros(gestor_voos_t *gestor, voo_t *voo);

/**
 * @brief Obtém um voo pelo identificador.
 *
 * @param gestor Gestor de voos.
 * @param flight_id Identificador do voo.
 * @return Ponteiro para o voo ou NULL se não existir.
 */
voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id);

/**
 * @brief Obtém um voo pela chave numérica.
 *
 * @param gestor Gestor de voos.
 * @param key Chave numérica do voo.
 * @return Ponteiro para o voo ou NULL se não existir.
 */
voo_t *gestor_voos_obter_por_key(gestor_voos_t *gestor, uint64_t key);

/**
 * @brief Retorna o número de voos no gestor.
 *
 * @param gestor Gestor de voos.
 * @return Número de voos.
 */
unsigned int gestor_voos_contar(const gestor_voos_t *gestor);

/**
 * @brief Carrega voos de um ficheiro CSV.
 *
 * @param gestor Gestor de voos.
 * @param ficheiro_csv Caminho do CSV.
 */
void gestor_voos_carregar(gestor_voos_t *gestor, const char *ficheiro_csv);

/**
 * @brief Carrega voos com validação de aeronaves.
 *
 * @param gestor Gestor de voos.
 * @param ficheiro_csv Caminho do CSV.
 * @param gestor_avioes Gestor de aviões para validação.
 */
void gestor_voos_carregar_com_validacao(gestor_voos_t *gestor, const char *ficheiro_csv, gestor_avioes_t *gestor_avioes);

/**
 * @brief Aplica um callback a cada voo.
 *
 * @param gestor Gestor de voos.
 * @param callback Função a executar por voo.
 * @param user_data Dados do utilizador.
 */
void gestor_voos_para_cada(gestor_voos_t *gestor, void (*callback)(voo_t *, void *), void *user_data);

/**
 * @brief Prepara contagens prefixadas para a Q3.
 *
 * @param gestor Gestor de voos.
 */
void gestor_voos_preparar_q3(gestor_voos_t *gestor);

/**
 * @brief Determina a melhor origem num intervalo de dias (Q3).
 *
 * @param gestor Gestor de voos.
 * @param dia_inicio Dia inicial.
 * @param dia_fim Dia final.
 * @param out_origem Output do código de origem.
 * @param out_contagem Output da contagem.
 * @return TRUE se houver resultado, FALSE caso contrário.
 */
gboolean gestor_voos_melhor_origem_intervalo(gestor_voos_t *gestor, int dia_inicio, int dia_fim, const char **out_origem, guint *out_contagem);

/**
 * @brief Itera estatísticas de atraso por companhia.
 *
 * @param gestor Gestor de voos.
 * @param callback Função de callback.
 * @param user_data Dados do utilizador.
 */
void gestor_voos_para_cada_atraso(gestor_voos_t *gestor, void (*callback)(const char *airline, guint count, double total_delay, void *), void *user_data);

/**
 * @brief Obtém o cache de atrasos para Q5.
 *
 * @param gestor Gestor de voos.
 * @return Array com estatísticas ordenadas.
 */
const GArray *gestor_voos_obter_q5_cache(gestor_voos_t *gestor);

/**
 * @brief Atualiza contagens de chegadas/partidas dos aeroportos.
 *
 * @param gestor_voos Gestor de voos.
 * @param gestor_aeroportos Gestor de aeroportos.
 */
void gestor_voos_atualizar_contagens_aeroportos(gestor_voos_t *gestor_voos, gestor_aeroportos_t *gestor_aeroportos);

#endif
