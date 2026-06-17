#include "gestores/gestor_passageiros.h"
#include "estruturas/passageiro_table.h"
#include "parsers/parser.h"
#include "validacoes/validacao_passageiros.h"
#include "utils.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Estrutura que representa um gestor de passageiros.
 *
 * Contém uma HashTable que mapeia números de documento para as
 * estruturas de passageiros correspondentes.
 */
struct gestor_passageiros
{
    passageiro_table_t *por_documento;
    char *ficheiro_csv;
    int dataset_grande;
};

/**
 * @brief Cria um gestor de passageiros.
 *
 * @return Ponteiro para o gestor criado.
 */
gestor_passageiros_t *gestor_passageiros_criar(void)
{
    gestor_passageiros_t *g = malloc(sizeof(*g));
    if (!g)
        return NULL;
    g->por_documento = passageiro_table_create(1 << 16);
    if (!g->por_documento)
    {
        free(g);
        return NULL;
    }
    g->ficheiro_csv = NULL;
    g->dataset_grande = 0;
    return g;
}

/**
 * @brief Destroi um gestor de passageiros e libera a memória associada.
 *
 * @param gestor Ponteiro para o gestor a ser destruído.
 */
void gestor_passageiros_destruir(gestor_passageiros_t *gestor)
{
    if (!gestor)
        return;
    passageiro_table_free(gestor->por_documento, passageiro_destruir);
    free(gestor->ficheiro_csv);
    free(gestor);
}

/**
 * @brief Adiciona um passageiro ao gestor.
 *
 * Se o passageiro já existir no gestor (mesmo número de documento),
 * é destruído e não substitui o existente.
 *
 * @param gestor Ponteiro para o gestor de passageiros.
 * @param p Ponteiro para o passageiro a ser adicionado.
 */
void gestor_passageiros_adicionar(gestor_passageiros_t *gestor, passageiro_t *p)
{
    if (!gestor || !p)
        return;

    uint32_t key = passageiro_obter_document_key(p);
    if (key == 0)
    {
        passageiro_destruir(p);
        return;
    }

    if (!passageiro_table_insert(gestor->por_documento, key, p))
        passageiro_destruir(p);
}

/**
 * @brief Obtém um passageiro pelo número de documento.
 *
 * @param gestor Ponteiro para o gestor de passageiros.
 * @param document_number Número do documento do passageiro.
 * @return Ponteiro para o passageiro correspondente, ou NULL se não encontrado.
 */
passageiro_t *gestor_passageiros_obter_por_documento(gestor_passageiros_t *gestor, const char *document_number)
{
    uint32_t key = 0;
    if (!gestor || !document_number || !utils_document_number_key(document_number, &key))
        return NULL;
    return gestor_passageiros_obter_por_documento_key(gestor, key);
}

passageiro_t *gestor_passageiros_obter_por_documento_key(gestor_passageiros_t *gestor, uint32_t key)
{
    if (!gestor)
        return NULL;
    return passageiro_table_lookup(gestor->por_documento, key);
}

/**
 * @brief Retorna o número de passageiros no gestor.
 *
 * @param gestor Ponteiro para o gestor de passageiros.
 * @return Número de passageiros armazenados.
 */
unsigned int gestor_passageiros_numero(gestor_passageiros_t *gestor)
{
    return gestor ? (unsigned int)passageiro_table_size(gestor->por_documento) : 0;
}

/**
 * @brief Callback interno usado para adicionar passageiros ao gestor durante o carregamento.
 *
 * @param contexto Ponteiro para o gestor de passageiros.
 * @param objeto Ponteiro para o passageiro a ser adicionado.
 * @return Sempre retorna TRUE para continuar a iteração.
 */
static gboolean adiciona_passageiro_callback(void *contexto, void *objeto)
{
    if (contexto && objeto)
        gestor_passageiros_adicionar(contexto, objeto);
    return TRUE;
}

/**
 * @brief Carrega passageiros de um ficheiro CSV e adiciona ao gestor.
 *
 * @param gestor Ponteiro para o gestor de passageiros.
 * @param ficheiro_csv Caminho para o ficheiro CSV contendo os passageiros.
 */
void gestor_passageiros_carregar(gestor_passageiros_t *gestor, const char *ficheiro_csv)
{
    if (!gestor || !ficheiro_csv)
        return;

    free(gestor->ficheiro_csv);
    gestor->ficheiro_csv = g_strdup(ficheiro_csv);
    gestor->dataset_grande = (strstr(ficheiro_csv, "grande") != NULL);
    if (gestor->dataset_grande)
        passageiro_table_reserve(gestor->por_documento, 1u << 22);
    // Em datasets grandes só precisamos das 5 primeiras colunas para a versão compacta.
    int colunas_necessarias = gestor->dataset_grande ? 5 : 10;

    parser_carrega(
        gestor,
        ficheiro_csv,
        adiciona_passageiro_callback,
        (LinhaParaObjeto)valida_passageiro,
        (DestroiObjeto)passageiro_destruir,
        10,
        colunas_necessarias
    );
}

void gestor_passageiros_carregar_detalhes(gestor_passageiros_t *gestor, GHashTable *doc_keys)
{
    if (!gestor || !doc_keys || !gestor->dataset_grande || !gestor->ficheiro_csv)
        return;

    FILE *ficheiro = fopen(gestor->ficheiro_csv, "r");
    if (!ficheiro)
        return;

    setvbuf(ficheiro, NULL, _IOFBF, 8 * 1024 * 1024);

    char *linha = NULL;
    size_t tamanho = 0;
    char *linha_parse = NULL;
    size_t tamanho_parse = 0;
    ssize_t lidos;

    (void)getline(&linha, &tamanho, ficheiro); /* cabeçalho */

    size_t restantes = g_hash_table_size(doc_keys);
    while (restantes > 0 && (lidos = getline(&linha, &tamanho, ficheiro)) != -1)
    {
        size_t necessario = (size_t)lidos + 1;
        if (necessario > tamanho_parse) {
            char *novo = realloc(linha_parse, necessario);
            if (!novo)
                break;
            linha_parse = novo;
            tamanho_parse = necessario;
        }
        memcpy(linha_parse, linha, necessario);

        char *colunas[12];
        int num = parser_dividir_csv_ate(linha_parse, colunas, 10, 4);
        if (num < 4)
            continue;

        utils_remove_aspas_somente(colunas[0]);
        utils_remove_aspas_somente(colunas[1]);
        utils_remove_aspas_somente(colunas[2]);
        utils_remove_aspas_somente(colunas[3]);

        uint32_t key = 0;
        if (!utils_document_number_key(colunas[0], &key))
            continue;

        gpointer kptr = GINT_TO_POINTER((gint)(key + 1u));
        if (!g_hash_table_contains(doc_keys, kptr))
            continue;

        passageiro_t *p = gestor_passageiros_obter_por_documento_key(gestor, key);
        if (p && !passageiro_tem_detalhes(p))
        {
            passageiro_definir_detalhes(p, colunas[1], colunas[2], colunas[3]);
            g_hash_table_remove(doc_keys, kptr);
            restantes--;
        }
    }

    free(linha_parse);
    free(linha);
    fclose(ficheiro);
}
