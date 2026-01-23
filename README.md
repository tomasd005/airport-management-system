# Airport Management System (LI3)

Sistema em C para gestão de aeroportos, voos, passageiros e reservas.  
Processa datasets CSV e executa um conjunto de queries, com modo principal, testes automáticos e modo interativo.

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

## Variáveis de ambiente úteis
- `LI3_USE_MMAP=1`  
  Ativa `mmap` no parser genérico (pode aumentar memória).
- `LI3_USE_MMAP_RESERVAS=1`  
  Ativa `mmap` apenas no parser de reservas.

Por defeito, o `mmap` está desativado para manter o consumo de memória baixo.

## Benchmarks em Docker
```
docker build -t li3-bench .
docker run --rm -v "$PWD/resultados:/app/resultados" li3-bench
```

Os ficheiros finais ficam em:
- `resultados/benchmark-regular.json` / `benchmark-regular.html`
- `resultados/benchmark-large.json` / `benchmark-large.html`

## Estrutura do projeto (resumo)
- `src/` — implementação
- `include/` — headers
- `resultados/` — outputs gerados (não recomendado versionar)
- `sem_erros/`, `com_erros/` — datasets (não recomendados para versionar)

## Nota sobre ficheiros grandes
Resultados e datasets podem ultrapassar o limite recomendado pelo GitHub.  
Mantém `resultados/` e datasets no `.gitignore` para evitar uploads acidentais.
