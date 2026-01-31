# Airport Management System (LI3)

Sistema em C para gestão de aeroportos, voos, passageiros e reservas.  
Processa datasets CSV e executa um conjunto de queries, com modo principal, testes automáticos e modo interativo.

Destaques técnicos:
- Parsing em streaming por blocos (sem carregar linhas completas em memória).
- Estruturas compactas para reduzir memória no dataset grande.
- Modo interativo com histórico, repetição e tempos de execução por query.

## Requisitos
- GCC (ou clang)
- `make`
- `glib-2.0` (dev)

Em Ubuntu:
```
sudo apt-get install build-essential pkg-config libglib2.0-dev
```

## Compilar
```
make
```

O Makefile usa otimizações agressivas por omissão (`FAST=1`).  
Para compilar sem essas otimizações:
```
make FAST=0
```

## Executar (modo principal)
```
./programa-principal <pasta_dados> <ficheiro_input>
```
Exemplo:
```
./programa-principal sem_erros inputs_fase2.txt
```

Os outputs são gerados em `resultados/`.

## Executar testes automáticos
```
./programa-testes <pasta_datasets> <ficheiro_input> <pasta_esperados>
```
Exemplos:
```
./programa-testes sem_erros inputs_fase2.txt resultados-esperados
./programa-testes sem_erros_grande inputs_fase2.txt resultados_esperados-grande
```

O programa de testes compara outputs e gera relatórios de desempenho em `resultados/benchmark.json` e `resultados/benchmark.html`.

## Modo interativo
```
./programa-interativo
```
Podes indicar o caminho do dataset na interface (por omissão usa `./dataset`).

Funcionalidades no modo interativo:
- Histórico de comandos
- Repetir a última query
- Mostrar detalhes da última query
- Tempos de execução por query

## Variáveis de ambiente úteis
- `LI3_SKIP_ERROR_LOG=1`  
  Desativa o log de erros CSV (reduz I/O em datasets grandes).
- `LI3_PARSE_THREADS=N` + `LI3_PARSE_UNSAFE=1`  
  Ativa parsing em paralelo com N threads (experimental; pode alterar resultados se o código
  usar estruturas globais não thread‑safe).

## Benchmarks em Docker
```
docker build -t li3-bench .
docker run --rm -v "$PWD/resultados:/app/resultados" li3-bench
```

Os ficheiros finais ficam em:
- `resultados/benchmark-regular.json` / `benchmark-regular.html`
- `resultados/benchmark-large.json` / `benchmark-large.html`
