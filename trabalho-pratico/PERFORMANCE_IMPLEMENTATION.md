# Implementacao das otimizacoes de performance

Objetivo medido: reduzir o dataset grande de ~11.25s para menos de 7s sem alterar outputs.

Resultado atual:

| Build | Comando | Tempo | RSS | Resultado |
| --- | --- | ---: | ---: | --- |
| baseline inicial | `./programa-principal ./sem_erros_grande inputs_fase2.txt` | 11.25s | 521 MB | referencia |
| benchmark final | `LI3_DISABLE_RESULT_CACHE=1 ./programa-principal ./sem_erros_grande inputs_fase2.txt` | 6.60s | 547 MB | OK |
| benchmark final + testes | `LI3_DISABLE_RESULT_CACHE=1 ./programa-testes ./sem_erros_grande inputs_fase2.txt resultados_esperados-grande` | 6.69s | 547 MB | 300/300 |

Ganho end-to-end medido: cerca de 41% no dataset grande.

## Garantias de modularidade

As alteracoes mantem encapsulamento:

- `query2.c` deixou de conhecer detalhes de `aviao_t`; consome apenas `gestor_avioes_obter_q2_ranking`.
- Reservas nao alteram campos internos de voos; usam `gestor_voos_registar_passageiros`.
- Tabelas compactas continuam opacas; foram acrescentadas APIs de reserva de capacidade.
- O fast path de parsing ficou dentro de `parser_reservas.c` e `validacao_reservas.c`.

## Otimizacoes implementadas

| Rank | Otimizacao | Ficheiros principais | Antes | Depois | Ganho estimado/medido | Risco |
| ---: | --- | --- | --- | --- | --- | --- |
| 1 | Parser de reservas `sem_erros` so divide 5 colunas | `src/parsers/parser_reservas.c`, `src/validacoes/validacao_reservas.c` | Varre ate 8 colunas por linha | Varre ate `price` | ~1.1s medidos | Baixo; caminho `com_erros` continua com 8 colunas |
| 2 | Q2 materializada | `src/gestores/gestor_avioes.c`, `src/queries/querie2.c` | O(A log A) por query | O(1) obter ranking + O(N) output | Alto nas 50 Q2 | Baixo |
| 3 | Hash tables power-of-two + mask | `src/estruturas/voo_table.c`, `src/estruturas/passageiro_table.c` | `% capacity`, load alto | `& mask`, load 78% | Medio | Medio-baixo; mais RAM |
| 4 | Iteracao compacta de voos existentes | `src/estruturas/voo_table.c` | O(capacidade) | O(voos) | Medio | Baixo |
| 5 | Q3 active origins | `src/gestores/gestor_voos.c` | 26^3 origens por query | Apenas origens observadas | Medio em Q3 | Baixo |
| 6 | Agregados de passageiros por aeroporto | `src/gestores/gestor_voos.c`, `src/gestores/gestor_reservas.c` | Scan final de todos os voos | Atualizacao incremental nas reservas | Medio | Baixo |
| 7 | Reserva antecipada de capacidade | `src/gestores/gestor_voos.c`, `src/gestores/gestor_passageiros.c` | Rehashes/reallocs durante carga | Capacidade grande prealocada | Pequeno-medio | Baixo; mais RSS |
| 8 | `MODE=benchmark` e PGO/LTO | `Makefile` | Sem alvo dedicado | `make MODE=benchmark`, `pgo-*` | `MODE=benchmark` pequeno; PGO sem ganho no teste | Baixo |
| 9 | `LI3_USE_MMAP=1` experimental | `src/parsers/parser.c`, `src/parsers/parser_reservas.c` | `read()` | `mmap()` opt-in | Nao melhorou; RSS muito maior | Baixo por estar desativado por defeito |

## Complexidade antes/depois

| Area | Antes | Depois |
| --- | --- | --- |
| Q2 | por query: O(A + A log A) + alocacao de `GArray` | uma construcao lazy O(A log A); por query O(N) |
| Q3 | preparacao O(V); query O(26^3) | preparacao O(V) mas prefix apenas ativo; query O(origens_ativas) |
| Iteracao de voos | O(capacidade da tabela) | O(numero de voos) |
| Reservas `sem_erros` parsing | O(bytes ate coluna 8) por linha | O(bytes ate coluna 5) por linha |
| Hash lookup voos/passageiros | open addressing com modulo | open addressing com mascara, menor load factor |
| Contagens de aeroportos | scan final O(V) | aplicacao final O(26^3), acumulacao O(reservas) ja no hot path |

## Memoria

| Estrutura | Alteracao | Impacto |
| --- | --- | --- |
| `voo_table` | `ids` compactos para iteracao | ~4 bytes por voo/capacidade reservada |
| `passageiro_table` | load factor menor e reserve | mais buckets, menos probes |
| Q2 index | rankings materializados | pequeno: 5000 avioes |
| Q3 active origins | vetor de indices ativos | pequeno |
| Agregados aeroportos | duas arrays `uint32_t[26^3]` | ~140 KB |

RSS final medido no dataset grande: ~547 MB, cerca de +26 MB face ao baseline inicial, dentro do objetivo de aceitar mais memoria por performance.

## Comandos de validacao

```sh
make MODE=benchmark
./programa-testes sem_erros inputs_fase2.txt resultados-esperados
./programa-testes com_erros inputs_fase2.txt resultados-esperados
LI3_DISABLE_RESULT_CACHE=1 ./programa-testes ./sem_erros_grande inputs_fase2.txt resultados_esperados-grande
/usr/bin/time -v env LI3_DISABLE_RESULT_CACHE=1 ./programa-principal ./sem_erros_grande inputs_fase2.txt
```

## Top 5 retorno por linha modificada

1. Parser de reservas com 5 colunas no `sem_erros`.
2. Q2 materializada no gestor de avioes.
3. `voo_table_foreach_id` sobre ids compactos.
4. `passageiro_table_insert` sem lookup duplicado no gestor.
5. `MODE=benchmark` com `-O3`, `-march=native`, `-mtune=native`, `-flto`.

## Trabalho nao implementado

- Cache de Q4 por intervalo: nao foi implementada porque Q4 ficou abaixo de 6 ms totais no dataset grande; o custo principal era carregamento/parsing.
- Paralelizacao das reservas: a implementacao existente serializa no `add_mutex`; uma versao correta exige acumuladores locais por thread e reducao final.
- `mmap` como default: medido como pior no dataset grande, ficando apenas opt-in por `LI3_USE_MMAP=1`.
