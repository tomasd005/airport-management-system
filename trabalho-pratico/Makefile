CC = gcc

# Otimizações (FAST=1 por omissão para máximo desempenho)
FAST ?= 1
ifeq ($(FAST),1)
OPT_FLAGS = -O3 -flto -pipe -march=native -mtune=native -fno-plt
else
OPT_FLAGS = -O2 -flto -pipe
endif
OPT_LDFLAGS = -flto

# Flags fixas (sempre usadas)
CFLAGS = -w $(OPT_FLAGS) -Wextra -g \
         -Iinclude \
         -Iinclude/entidades \
         -Iinclude/gestores \
         -Iinclude/gestor_programa \
         -Iinclude/gestor_programas \
         -Iinclude/parsers \
         -Iinclude/queries \
         -Iinclude/validacoes \
         $(shell pkg-config --cflags glib-2.0)

LDFLAGS = $(OPT_LDFLAGS) $(shell pkg-config --libs glib-2.0)

# Número de jobs paralelos (usa todos os cores)
MAKEFLAGS += -j$(shell nproc)

# Entidades
SRC_ENTIDADES = \
    src/entidades/aeroportos.c \
    src/entidades/avioes.c \
    src/entidades/passageiros.c \
    src/entidades/reservas.c \
    src/entidades/voos.c

# Gestores individuais
SRC_GESTORES = \
    src/gestores/gestor_aeroportos.c \
    src/gestores/gestor_avioes.c \
    src/gestores/gestor_voos.c \
    src/gestores/gestor_passageiros.c \
    src/gestores/gestor_reservas.c

# Gestor do programa principal
SRC_GESTOR_PROGRAMA = \
    src/gestor_programa/gestor_programa.c \
    src/gestor_programa/gestor_queries.c

# Gestores de programas (principal, teste, interativo)
SRC_GESTOR_PROGRAMAS = \
    src/gestor_programas/gestor_principal.c \
    src/gestor_programas/gestor_teste.c \
    src/gestor_programas/gestor_interativo.c

# Parsers e validacoes
SRC_PARSERS = \
    src/parsers/parser.c \
    src/parsers/parser_reservas.c \
    src/validacoes/validacao_comum.c \
    src/validacoes/validacao_aeroportos.c \
    src/validacoes/validacao_avioes.c \
    src/validacoes/validacao_voos.c \
    src/validacoes/validacao_passageiros.c \
    src/validacoes/validacao_reservas.c

# Queries
SRC_QUERIES = \
    src/queries/querie1.c \
    src/queries/querie2.c \
    src/queries/querie3.c \
    src/queries/querie4.c \
    src/queries/querie5.c \
    src/queries/querie6.c

SRC_UTILS = src/utils.c
SRC_ESTRUTURAS = src/estruturas/voo_table.c src/estruturas/doc_total_table.c

# Programas principais
MAIN_PRINCIPAL = src/programas/main.c
MAIN_TESTES = src/programas/teste.c
MAIN_VALIDACOES = src/programas/test_validacoes.c
MAIN_INTERATIVO = src/programas/interativo.c

# Executáveis
EXEC_PRINCIPAL = programa-principal
EXEC_TESTES = programa-testes
EXEC_VALIDACOES = test_validacoes
EXEC_INTERATIVO = programa-interativo

# Targets principais
all: $(EXEC_PRINCIPAL) $(EXEC_TESTES) $(EXEC_INTERATIVO)

$(EXEC_PRINCIPAL): $(MAIN_PRINCIPAL) $(SRC_GESTOR_PROGRAMA) $(SRC_GESTORES) \
$(SRC_ENTIDADES) $(SRC_PARSERS) $(SRC_QUERIES) $(SRC_UTILS) $(SRC_ESTRUTURAS)
	@echo "Compilando $(EXEC_PRINCIPAL)..."
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EXEC_TESTES): $(MAIN_TESTES) $(SRC_GESTOR_PROGRAMAS) $(SRC_GESTORES) $(SRC_ENTIDADES) \
$(SRC_PARSERS) $(SRC_QUERIES) $(SRC_UTILS) $(SRC_ESTRUTURAS)
	@echo "Compilando $(EXEC_TESTES)..."
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EXEC_INTERATIVO): $(MAIN_INTERATIVO) $(SRC_GESTOR_PROGRAMAS) $(SRC_GESTORES) $(SRC_ENTIDADES) \
$(SRC_PARSERS) $(SRC_QUERIES) $(SRC_UTILS) $(SRC_ESTRUTURAS)
	@echo "Compilando $(EXEC_INTERATIVO)..."
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EXEC_VALIDACOES): $(MAIN_VALIDACOES) $(SRC_ENTIDADES) $(SRC_PARSERS) $(SRC_UTILS)
	@echo "Compilando $(EXEC_VALIDACOES)..."
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@


# Limpeza
clean:
	@echo "A limpar ficheiros compilados..."
	rm -f $(EXEC_PRINCIPAL) $(EXEC_TESTES) $(EXEC_INTERATIVO) $(EXEC_VALIDACOES)
	find src -type f -name "*.o" -delete
	rm -f resultados/*.txt resultados/*_errors.csv

# Execuções rápidas ou com Valgrind
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1
TIME = /usr/bin/time -v

run-erros: $(EXEC_PRINCIPAL)
	@echo "Executando com dataset COM_ERROS (Valgrind + Tempo)..."
	$(TIME) $(VALGRIND) ./$(EXEC_PRINCIPAL) com_erros/ inputs_fase2.txt

run-sem-erros: $(EXEC_PRINCIPAL)
	@echo "Executando com dataset SEM_ERROS (Valgrind + Tempo)..."
	$(TIME) $(VALGRIND) ./$(EXEC_PRINCIPAL) sem_erros/ inputs_fase2.txt

run-fast: $(EXEC_PRINCIPAL)
	@echo "Execução rápida SEM Valgrind..."
	$(TIME) ./$(EXEC_PRINCIPAL) sem_erros/ inputs_fase2.txt

.PHONY: all clean run-erros run-sem-erros run-fast
