CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c17 \
         -Iinclude -Iinclude/entidades -Iinclude/gestores \
         -Iinclude/parsers -Iinclude/queries -Iinclude/validacoes \
         $(shell pkg-config --cflags glib-2.0)
LDFLAGS = $(shell pkg-config --libs glib-2.0)

SRC_ENTIDADES = \
    src/entidades/aeroportos.c \
    src/entidades/avioes.c \
    src/entidades/passageiros.c \
    src/entidades/reservas.c \
    src/entidades/voos.c

SRC_GESTORES = \
    src/gestores/gestor_aeroportos.c \
    src/gestores/gestor_avioes.c \
    src/gestores/gestor_voos.c \
    src/gestores/gestor_passageiros.c \
    src/gestores/gestor_reservas.c

SRC_PARSERS = \
    src/parsers/parser.c \
    src/validacoes/validacao_comum.c \
    src/validacoes/validacao_aeroportos.c \
    src/validacoes/validacao_avioes.c \
    src/validacoes/validacao_voos.c \
    src/validacoes/validacao_passageiros.c \
    src/validacoes/validacao_reservas.c

SRC_QUERIES = \
    src/queries/querie1.c \
    src/queries/querie2.c \
    src/queries/querie3.c

SRC_UTILS = src/utils.c

MAIN_PRINCIPAL = src/programas/main.c
MAIN_TESTES = src/programas/teste.c
MAIN_VALIDACOES = src/programas/test_validacoes.c

EXEC_PRINCIPAL = programa-principal
EXEC_TESTES = programa-testes
EXEC_VALIDACOES = test_validacoes
EXEC_VERIFICADOR = verificador-csv

all: $(EXEC_PRINCIPAL) $(EXEC_TESTES)

$(EXEC_PRINCIPAL): $(MAIN_PRINCIPAL) $(SRC_GESTORES) $(SRC_ENTIDADES) $(SRC_PARSERS) $(SRC_QUERIES) $(SRC_UTILS)
	@echo " Compilando $(EXEC_PRINCIPAL)..."
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EXEC_TESTES): $(MAIN_TESTES) $(SRC_GESTORES) $(SRC_ENTIDADES) $(SRC_PARSERS) $(SRC_QUERIES) $(SRC_UTILS)
	@echo " Compilando $(EXEC_TESTES)..."
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(EXEC_VALIDACOES): $(MAIN_VALIDACOES) $(SRC_ENTIDADES) $(SRC_PARSERS) $(SRC_UTILS)
	@echo " Compilando $(EXEC_VALIDACOES)..."
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# Compilar verificador CSV (sem -Werror para compatibilidade)
$(EXEC_VERIFICADOR): verificador-csv.c
	@echo " Compilando $(EXEC_VERIFICADOR)..."
	$(CC) -Wall -Wextra -std=c17 $(shell pkg-config --cflags glib-2.0) $< $(LDFLAGS) -o $@

clean:
	@echo " A limpar ficheiros compilados..."
	rm -f $(EXEC_PRINCIPAL) $(EXEC_TESTES) $(EXEC_VALIDACOES) $(EXEC_VERIFICADOR)
	find src -type f -name "*.o" -delete
	rm -f resultados/*.txt resultados/*_errors.csv

VALGRIND = valgrind --leak-check=full --show-leak-kinds=all \
           --track-origins=yes --error-exitcode=1
TIME = /usr/bin/time -v

run-erros: $(EXEC_PRINCIPAL)
	@echo " Executando com dataset COM_ERROS (Valgrind + Tempo)..."
	$(TIME) $(VALGRIND) ./$(EXEC_PRINCIPAL) com_erros/ inputs_fase1.txt

run-sem-erros: $(EXEC_PRINCIPAL)
	@echo "  Executando com dataset SEM_ERROS (Valgrind + Tempo)..."
	$(TIME) $(VALGRIND) ./$(EXEC_PRINCIPAL) sem_erros/ inputs_fase1.txt

run-testes: $(EXEC_TESTES)
	@echo "  Executando programa de TESTES (Valgrind + Tempo)..."
	$(TIME) $(VALGRIND) ./$(EXEC_TESTES) sem_erros/ inputs_fase1.txt resultados-esperados/

run-fast: $(EXEC_PRINCIPAL)
	@echo " Execução rápida SEM Valgrind..."
	$(TIME) ./$(EXEC_PRINCIPAL) sem_erros/ inputs_fase1.txt

test-fast: $(EXEC_TESTES)
	@echo " Testes rápidos SEM Valgrind..."
	./$(EXEC_TESTES) sem_erros/ inputs_fase1.txt resultados-esperados/

# Novo target para testes de validação
test-validacoes: $(EXEC_VALIDACOES)
	@echo " Executando testes de validação..."
	@echo ""
	./$(EXEC_VALIDACOES)

# Compilar testes de validação
build-validacoes: $(EXEC_VALIDACOES)
	@echo " Testes de validação compilados!"

# Executar com valgrind
test-validacoes-valgrind: $(EXEC_VALIDACOES)
	@echo " Executando testes de validação com Valgrind..."
	$(VALGRIND) ./$(EXEC_VALIDACOES)

# ===== NOVOS TARGETS PARA VERIFICADOR =====

# Compilar apenas o verificador
verificador: $(EXEC_VERIFICADOR)
	@echo " Verificador CSV compilado!"

# Verificar validações com dataset COM erros
verificar-erros: $(EXEC_PRINCIPAL) $(EXEC_VERIFICADOR)
	@echo "A executar programa com dataset COM_ERROS..."
	@./$(EXEC_PRINCIPAL) com_erros/ inputs_fase1.txt
	@echo ""
	@echo " A verificar ficheiros de erros gerados..."
	@./$(EXEC_VERIFICADOR) resultados/ com_erros/

# Verificar validações com dataset SEM erros
verificar-sem-erros: $(EXEC_PRINCIPAL) $(EXEC_VERIFICADOR)
	@echo " A executar programa com dataset SEM_ERROS..."
	@./$(EXEC_PRINCIPAL) sem_erros/ inputs_fase1.txt
	@echo ""
	@echo "A verificar resultados..."
	@./$(EXEC_VERIFICADOR) resultados/ sem_erros/

# Verificação completa (COM e SEM erros)
verificar-tudo: $(EXEC_PRINCIPAL) $(EXEC_VERIFICADOR)
	@echo "════════════════════════════════════════"
	@echo " VERIFICAÇÃO COM ERROS"
	@echo "════════════════════════════════════════"
	@./$(EXEC_PRINCIPAL) com_erros/ inputs_fase1.txt
	@./$(EXEC_VERIFICADOR) resultados/ com_erros/
	@echo ""
	@echo "════════════════════════════════════════"
	@echo " VERIFICAÇÃO SEM ERROS"
	@echo "════════════════════════════════════════"
	@rm -f resultados/*
	@./$(EXEC_PRINCIPAL) sem_erros/ inputs_fase1.txt
	@./$(EXEC_VERIFICADOR) resultados/ sem_erros/

# Workflow completo: compilar, executar e verificar
workflow: clean all verificar-erros
	@echo " Workflow completo executado!"

debug: CFLAGS += -g -O0
debug: clean all
	@echo " Compilação em modo DEBUG concluída."

.PHONY: all clean run-erros run-sem-erros run-testes run-fast test-fast debug \
        test-validacoes build-validacoes test-validacoes-valgrind \
        verificador verificar-erros verificar-sem-erros verificar-tudo workflow