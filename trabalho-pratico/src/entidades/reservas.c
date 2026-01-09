#include "reservas.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdbool.h>

/**
 * @struct reserva
 * @brief Representa uma reserva de voo, associando um passageiro a um ou mais voos.
 */
struct reserva
{
    char *reservation_id;    
    char **flight_ids;       
    size_t num_voos;         
    char *document_number;  
    double preco;            
};

/**
 * @brief Cria uma nova reserva.
 *
 * @param reservation_id ID da reserva
 * @param flight_ids Array de IDs de voos
 * @param num_flights Número de voos
 * @param document_number Número do documento do passageiro
 * @param seat Lugar
 * @param extra_bagagem Indica bagagem extra
 * @param embarque_prioritario Indica embarque prioritário
 * @param qr_code Código QR
 * @return Ponteiro para reserva_t recém-criada ou NULL em caso de erro
 */
reserva_t *reserva_criar(const char *reservation_id, const char **flight_ids,
                         size_t num_flights, const char *document_number,
                         const char *seat, double preco, bool extra_bagagem,
                         bool embarque_prioritario, const char *qr_code)
{
    if (!reservation_id || !flight_ids || num_flights == 0 || !document_number)
        return NULL;

    reserva_t *r = malloc(sizeof(reserva_t));
    if (!r)
        return NULL;

    r->flight_ids = NULL;
    r->num_voos = 0;
    r->reservation_id = NULL;
    r->document_number = NULL;

    r->reservation_id = g_strdup(reservation_id);
    r->document_number = g_strdup(document_number);
    r->preco = preco;
    r->num_voos = num_flights;

    r->flight_ids = malloc(sizeof(char *) * num_flights);
    if (!r->flight_ids)
    {
        reserva_destruir(r);
        return NULL;
    }

    for (size_t i = 0; i < num_flights; i++)
    {
        r->flight_ids[i] = g_strdup(flight_ids[i]);
        if (!r->flight_ids[i])
        {
            reserva_destruir(r);
            return NULL;
        }
    }

    if (!r->reservation_id || !r->document_number)
    {
        reserva_destruir(r);
        return NULL;
    }

    return r;
}

/**
 * @brief Libera toda a memória associada a uma reserva.
 * @param r Ponteiro para reserva_t a ser destruída
 */
void reserva_destruir(reserva_t *r)
{
    if (!r)
        return;

    if (r->reservation_id)
        g_free(r->reservation_id);
    if (r->document_number)
        g_free(r->document_number);

    if (r->flight_ids)
    {
        for (size_t i = 0; i < r->num_voos; i++)
        {
            if (r->flight_ids[i])
                g_free(r->flight_ids[i]);
        }
        free(r->flight_ids);
    }

    free(r);
}

/**
 * @brief Obtém o ID da reserva.
 * @param r Reserva
 * @return ID da reserva ou NULL se r for NULL
 */
const char *reserva_obter_id(const reserva_t *r)
{
    return r ? r->reservation_id : NULL;
}

/**
 * @brief Obtém o número de voos da reserva.
 * @param r Reserva
 * @return Número de voos ou 0 se r for NULL
 */
size_t reserva_obter_num_voos(const reserva_t *r)
{
    return r ? r->num_voos : 0;
}

/**
 * @brief Obtém o array de IDs de voos da reserva.
 * @param r Reserva
 * @return Array de strings (const char**) ou NULL se r for NULL
 */
const char **reserva_obter_flight_ids(const reserva_t *r)
{
    return r ? (const char **)r->flight_ids : NULL;
}

/**
 * @brief Obtém o número de documento do passageiro da reserva.
 * @param r Reserva
 * @return Número de documento ou NULL se r for NULL
 */
const char *reserva_obter_document_number(const reserva_t *r)
{
    return r ? r->document_number : NULL;
}

/**
 * @brief Obtém o assento da reserva
 * @param r Reserva
 * @return Sempre string vazia se r não for NULL, caso contrário NULL
 */
const char *reserva_obter_seat(const reserva_t *r)
{
    return r ? "" : NULL;
}

/**
 * @brief Obtém o preço da reserva.
 * @param r Reserva
 * @return Preço ou 0.0 se r for NULL
 */
double reserva_obter_preco(const reserva_t *r)
{
    return r ? r->preco : 0.0;
}

/**
 * @brief Indica se há bagagem extra
 * @param r Reserva
 * @return Sempre false
 */
bool reserva_obter_extra_bagagem(const reserva_t *r)
{
    return false;
}

/**
 * @brief Indica se há embarque prioritário
 * @param r Reserva
 * @return Sempre false
 */
bool reserva_obter_embarque_prioritario(const reserva_t *r)
{
    return false;
}

/**
 * @brief Obtém o código QR da reserva
 * @param r Reserva
 * @return Sempre string vazia se r não for NULL, caso contrário NULL
 */
const char *reserva_obter_qr_code(const reserva_t *r)
{
    return r ? "" : NULL;
}

/**
 * @brief Obtém o número de passageiros da reserva.
 * @param r Reserva
 * @return Sempre 1 se r não for NULL, caso contrário 0
 */
size_t reserva_obter_num_passageiros(const reserva_t *r)
{
    return r ? 1 : 0;
}

/**
 * @brief Obtém um array com os documentos dos passageiros da reserva.
 * @param r Reserva
 * @return Array de strings com tamanho 1, ou NULL se r for NULL
 */
const char **reserva_obter_documentos(const reserva_t *r)
{
    if (!r)
        return NULL;

    static const char *docs[1];
    docs[0] = r->document_number;
    return docs;
}
