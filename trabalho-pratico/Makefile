# Makefile - build/projeto LI3
SHELL := /bin/sh
MAKEFLAGS += --warn-undefined-variables

CC ?= gcc
PKG_CONFIG ?= pkg-config

# Build modes: release (default), debug, sanitize
MODE ?= release
NATIVE ?= 1
LTO ?= 1

# Common flags
CPPFLAGS := -D_GNU_SOURCE \
	-Iinclude \
	-Iinclude/entidades \
	-Iinclude/gestores \
	-Iinclude/gestor_programa \
	-Iinclude/gestor_programas \
	-Iinclude/parsers \
	-Iinclude/queries \
	-Iinclude/validacoes

GLIB_CFLAGS := $(shell $(PKG_CONFIG) --cflags glib-2.0 2>/dev/null)
GLIB_LIBS := $(shell $(PKG_CONFIG) --libs glib-2.0 2>/dev/null)

ifeq ($(strip $(GLIB_CFLAGS)),)
$(error glib-2.0 não encontrado. Instale libglib2.0-dev e pkg-config.)
endif

CPPFLAGS += $(GLIB_CFLAGS)

WARN_FLAGS := -Wall -Wextra -Wpedantic -Wno-unused-parameter
CSTD := -std=c11
LDFLAGS :=

CFLAGS_DEBUG := $(CSTD) -O0 -g3 $(WARN_FLAGS) -fno-omit-frame-pointer
CFLAGS_RELEASE := $(CSTD) -O3 -DNDEBUG $(WARN_FLAGS)
CFLAGS_SANITIZE := $(CSTD) -O1 -g3 $(WARN_FLAGS) -fno-omit-frame-pointer \
	-fsanitize=address,undefined

ifeq ($(NATIVE),1)
CFLAGS_RELEASE += -march=native -mtune=native
endif

ifeq ($(LTO),1)
CFLAGS_RELEASE += -flto=auto
LDFLAGS += -flto=auto
endif

LDLIBS := $(GLIB_LIBS)

ifeq ($(MODE),release)
CFLAGS := $(CFLAGS_RELEASE)
else ifeq ($(MODE),sanitize)
CFLAGS := $(CFLAGS_SANITIZE)
LDFLAGS += -fsanitize=address,undefined
else
CFLAGS := $(CFLAGS_DEBUG)
endif

# Incremental build
OBJDIR := build/obj
DEPDIR := $(OBJDIR)

# Sources
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

SRC_GESTOR_PROGRAMA = \
	src/gestor_programa/gestor_programa.c \
	src/gestor_programa/gestor_queries.c

SRC_GESTOR_PROGRAMAS = \
	src/gestor_programas/gestor_principal.c \
	src/gestor_programas/gestor_teste.c \
	src/gestor_programas/gestor_interativo.c

SRC_PARSERS = \
	src/parsers/parser.c \
	src/parsers/parser_reservas.c \
	src/validacoes/validacao_comum.c \
	src/validacoes/validacao_aeroportos.c \
	src/validacoes/validacao_avioes.c \
	src/validacoes/validacao_voos.c \
	src/validacoes/validacao_passageiros.c \
	src/validacoes/validacao_reservas.c

SRC_QUERIES = \
	src/queries/querie1.c \
	src/queries/querie2.c \
	src/queries/querie3.c \
	src/queries/querie4.c \
	src/queries/querie5.c \
	src/queries/querie6.c

SRC_UTILS = src/utils.c src/string_pool.c
SRC_ESTRUTURAS = src/estruturas/voo_table.c src/estruturas/doc_total_table.c src/estruturas/passageiro_table.c

SRC_CORE = $(SRC_ENTIDADES) $(SRC_GESTORES) $(SRC_PARSERS) $(SRC_QUERIES) $(SRC_UTILS) $(SRC_ESTRUTURAS)

# Main programs
MAIN_PRINCIPAL = src/programas/main.c
MAIN_TESTES = src/programas/teste.c
MAIN_VALIDACOES = src/programas/test_validacoes.c
MAIN_INTERATIVO = src/programas/interativo.c

EXEC_PRINCIPAL = programa-principal
EXEC_TESTES = programa-testes
EXEC_VALIDACOES = test_validacoes
EXEC_INTERATIVO = programa-interativo

OBJ_CORE = $(addprefix $(OBJDIR)/,$(SRC_CORE:.c=.o))
OBJ_GESTOR_PROGRAMA = $(addprefix $(OBJDIR)/,$(SRC_GESTOR_PROGRAMA:.c=.o))
OBJ_GESTOR_PROGRAMAS = $(addprefix $(OBJDIR)/,$(SRC_GESTOR_PROGRAMAS:.c=.o))

OBJ_MAIN_PRINCIPAL = $(OBJDIR)/$(MAIN_PRINCIPAL:.c=.o)
OBJ_MAIN_TESTES = $(OBJDIR)/$(MAIN_TESTES:.c=.o)
OBJ_MAIN_VALIDACOES = $(OBJDIR)/$(MAIN_VALIDACOES:.c=.o)
OBJ_MAIN_INTERATIVO = $(OBJDIR)/$(MAIN_INTERATIVO:.c=.o)

# Default target
all: $(EXEC_PRINCIPAL) $(EXEC_TESTES)

# Build rules
$(EXEC_PRINCIPAL): $(OBJ_MAIN_PRINCIPAL) $(OBJ_GESTOR_PROGRAMA) $(OBJ_CORE)
	@echo "LD  $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXEC_TESTES): $(OBJ_MAIN_TESTES) $(OBJ_GESTOR_PROGRAMAS) $(OBJ_CORE)
	@echo "LD  $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXEC_INTERATIVO): $(OBJ_MAIN_INTERATIVO) $(OBJ_GESTOR_PROGRAMAS) $(OBJ_CORE)
	@echo "LD  $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXEC_VALIDACOES): $(OBJ_MAIN_VALIDACOES) $(OBJ_CORE)
	@echo "LD  $@"
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile objects with deps
$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "CC  $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJ_CORE:.o=.d) $(OBJ_GESTOR_PROGRAMA:.o=.d) $(OBJ_GESTOR_PROGRAMAS:.o=.d) \
	$(OBJ_MAIN_PRINCIPAL:.o=.d) $(OBJ_MAIN_TESTES:.o=.d) $(OBJ_MAIN_VALIDACOES:.o=.d) \
	$(OBJ_MAIN_INTERATIVO:.o=.d)

# Convenience targets
interativo: $(EXEC_INTERATIVO)
validacoes: $(EXEC_VALIDACOES)

release:
	$(MAKE) MODE=release

debug:
	$(MAKE) MODE=debug

sanitize:
	$(MAKE) MODE=sanitize

clean:
	@echo "CLEAN"
	@rm -rf build $(EXEC_PRINCIPAL) $(EXEC_TESTES) $(EXEC_INTERATIVO) $(EXEC_VALIDACOES)

# Test/bench helpers
TEST_DATASET ?= sem_erros
TEST_INPUT ?= inputs_fase2.txt

run: $(EXEC_PRINCIPAL)
	./$(EXEC_PRINCIPAL) $(TEST_DATASET)/ $(TEST_INPUT)

test: $(EXEC_TESTES)
	./$(EXEC_TESTES) $(TEST_DATASET)/ $(TEST_INPUT)

VALGRIND ?= valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1
TIME ?= /usr/bin/time -v

run-valgrind: $(EXEC_PRINCIPAL)
	$(TIME) $(VALGRIND) ./$(EXEC_PRINCIPAL) $(TEST_DATASET)/ $(TEST_INPUT)

format:
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i $(shell rg --files -g '*.c' -g '*.h' src include); \
	else \
		echo "clang-format não encontrado"; \
	fi

.PHONY: all clean interativo validacoes release debug sanitize run test run-valgrind format
