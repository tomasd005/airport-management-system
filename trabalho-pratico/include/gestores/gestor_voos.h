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

gestor_voos_t *gestor_voos_criar(void);
void gestor_voos_destruir(gestor_voos_t *gestor);
void gestor_voos_adicionar(gestor_voos_t *gestor, voo_info_t *info);
voo_t *gestor_voos_obter_por_id(gestor_voos_t *gestor, const char *flight_id);
voo_t *gestor_voos_obter_por_key(gestor_voos_t *gestor, uint64_t key);
unsigned int gestor_voos_contar(const gestor_voos_t *gestor);
void gestor_voos_carregar(gestor_voos_t *gestor, const char *ficheiro_csv);
void gestor_voos_carregar_com_validacao(gestor_voos_t *gestor, const char *ficheiro_csv, gestor_avioes_t *gestor_avioes);

/**
 * @brief Adiciona voo já validado, aplicando validação lógica extra (aeronave).
 *
 * @param gestor Gestor de voos
 * @param gestor_avioes Gestor de aviões
 * @param info Estrutura de voo já parseada/validada
 * @return TRUE se adicionado com sucesso
 */
gboolean gestor_voos_adicionar_validado(gestor_voos_t *gestor, gestor_avioes_t *gestor_avioes,
                                        voo_info_t *info);
void gestor_voos_para_cada(gestor_voos_t *gestor, void (*callback)(voo_t *, void *), void *user_data);
void gestor_voos_preparar_q3(gestor_voos_t *gestor);
gboolean gestor_voos_melhor_origem_intervalo(gestor_voos_t *gestor, int dia_inicio, int dia_fim, const char **out_origem, guint *out_contagem);
void gestor_voos_para_cada_atraso(gestor_voos_t *gestor, void (*callback)(const char *airline, guint count, double total_delay, void *), void *user_data);
const GArray *gestor_voos_obter_q5_cache(gestor_voos_t *gestor);
void gestor_voos_atualizar_contagens_aeroportos(gestor_voos_t *gestor_voos, gestor_aeroportos_t *gestor_aeroportos);

#endif
