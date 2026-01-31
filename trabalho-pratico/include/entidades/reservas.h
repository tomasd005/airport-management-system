#ifndef RESERVAS_H
#define RESERVAS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @file reservas.h
 * @brief Interface para manipulação de reservas.
 *
 * Define a estrutura e funções para criar, destruir e obter informações
 * sobre reservas de voos, incluindo voos associados, passageiro, assento,
 * preço, bagagem extra, embarque prioritário e código QR.
 */

/**
 * @brief Estrutura opaca que representa uma reserva.
 */
typedef struct reserva reserva_t;

/**
 * @brief Cria uma nova reserva.
 *
 * @param reservation_id Identificador único da reserva (ex: "R000000001").
 * @param flight_ids Array de IDs dos voos associados à reserva.
 * @param num_flights Número de voos na reserva (1 ou 2).
 * @param document_number Número do documento do passageiro.
 * @param seat Assento atribuído.
 * @param preco Preço da reserva.
 * @param extra_bagagem Indica se há bagagem extra.
 * @param embarque_prioritario Indica se há embarque prioritário.
 * @param qr_code Código QR da reserva.
 * @return Ponteiro para a reserva criada ou NULL em caso de erro.
 */
reserva_t *reserva_criar(const char *reservation_id, const char **flight_ids, size_t num_flights, const char *document_number, const char *seat, double preco, bool extra_bagagem, bool embarque_prioritario, const char *qr_code);

/**
 * @brief Destrói uma reserva e libera a memória associada.
 *
 * @param r Ponteiro para a reserva a destruir.
 */
void reserva_destruir(reserva_t *r);

/**
 * @brief Obtém o ID da reserva.
 * @param r Reserva.
 * @return String constante com o ID.
 */
const char *reserva_obter_id(const reserva_t *r);

/**
 * @brief Obtém o número de voos associados.
 * @param r Reserva.
 * @return Número de voos.
 */
size_t reserva_obter_num_voos(const reserva_t *r);

/**
 * @brief Obtém os IDs dos voos associados.
 * @param r Reserva.
 * @return Array de strings constantes.
 */
const char **reserva_obter_flight_ids(const reserva_t *r);

/**
 * @brief Obtém o número de documento do passageiro.
 * @param r Reserva.
 * @return String constante com o documento.
 */
const char *reserva_obter_document_number(const reserva_t *r);

/**
 * @brief Obtém o assento atribuído.
 * @param r Reserva.
 * @return String constante com o assento.
 */
const char *reserva_obter_seat(const reserva_t *r);

/**
 * @brief Obtém o preço da reserva.
 * @param r Reserva.
 * @return Preço.
 */
double reserva_obter_preco(const reserva_t *r);

/**
 * @brief Indica se há bagagem extra.
 * @param r Reserva.
 * @return TRUE se houver bagagem extra.
 */
bool reserva_obter_extra_bagagem(const reserva_t *r);

/**
 * @brief Indica se há embarque prioritário.
 * @param r Reserva.
 * @return TRUE se houver embarque prioritário.
 */
bool reserva_obter_embarque_prioritario(const reserva_t *r);

/**
 * @brief Obtém o código QR.
 * @param r Reserva.
 * @return String constante com o QR.
 */
const char *reserva_obter_qr_code(const reserva_t *r);
/* Para query6 */
/**
 * @brief Obtém o número de passageiros associados (query 6).
 * @param r Reserva.
 * @return Número de passageiros.
 */
size_t reserva_obter_num_passageiros(const reserva_t *r);

/**
 * @brief Obtém documentos associados (query 6).
 * @param r Reserva.
 * @return Array de documentos.
 */
const char **reserva_obter_documentos(const reserva_t *r);

#endif
