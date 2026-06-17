# Auditoria de performance

Contexto medido nesta auditoria:

- Build atual: `make MODE=release` ja estava atualizado.
- Dataset pequeno: `./programa-testes ./sem_erros inputs_fase2.txt resultados-esperados`
  - `programa-principal`: 1.04 s
  - RSS pico: 115.9 MB
  - 300/300 outputs corretos
- Dataset grande: `./programa-principal ./sem_erros_grande inputs_fase2.txt`
  - Wall clock: 11.25 s
  - User: 10.11 s
  - System: 1.71 s
  - CPU: 105%
  - RSS pico: 521 MB

Nota critica: `src/programas/main.c` tem cache persistente de outputs em
`resultados/.cache`, baseada em pasta/input/tamanho/mtime. Isto e util para
repeticoes identicas, mas mascara benchmarks de carga real. Medicoes devem
separar "cold run" e "cached run".

## Arquitetura e fluxo de dados

Fluxo atual:

1. `src/programas/main.c` cria `resultados`, tenta restaurar cache, cria
   `GestorDePrograma`.
2. `src/gestor_programa/gestor_programa.c` carrega:
   - aeroportos
   - avioes
   - voos e passageiros em paralelo com duas threads
   - reservas depois de voos/passageiros
3. Pos-processamento:
   - `gestor_reservas_finalizar`
   - segunda passagem parcial em passageiros para detalhes dos top10
   - `gestor_voos_atualizar_contagens_aeroportos`
   - `gestor_voos_preparar_q3`
   - `gestor_voos_descartar_tabela`
4. `gestor_queries_processar_ficheiro` abre um ficheiro de output por comando
   e chama Q1..Q6.

Dependencias principais:

- reservas dependem de voos e passageiros.
- voos dependem de avioes para validacao logica.
- Q1 usa aeroportos ja agregados por `gestor_voos_atualizar_contagens_aeroportos`.
- Q2 usa avioes com `contagem_voos`.
- Q3 usa prefix sums em `gestor_voos`.
- Q4 usa top10 semanal em `gestor_reservas` e detalhes seletivos de passageiros.
- Q5 usa cache ordenada de atrasos por companhia.
- Q6 usa melhor destino por nacionalidade materializado.

Hot paths estimados:

- CRITICO: parsing/validacao de `flights.csv` e `reservations.csv`.
- CRITICO: lookups em `gestor_voos_obter_por_key` durante reservas.
- ALTO IMPACTO: `gestor_voos_preparar_q3` + `gestor_voos_atualizar_contagens_aeroportos`
  fazem passagens completas separadas sobre todos os voos.
- ALTO IMPACTO: Q2 faz scan e sort de todos os avioes em todas as queries.
- ALTO IMPACTO: Q4 cria uma hash table temporaria em todas as queries.
- MEDIO IMPACTO: `gestor_queries_processar_ficheiro` faz `g_strdup`, `strtok_r`,
  `fopen` e `fclose` por comando.
- MEDIO IMPACTO: segunda leitura de passageiros em
  `gestor_passageiros_carregar_detalhes`.

## Problemas por impacto

### 1. CRITICO - cache persistente invalida benchmarks reais

Ficheiro: `src/programas/main.c`

Problema:

- `restaurar_cache_resultados` pode devolver outputs sem carregar CSV nem
  executar queries.
- O comportamento funcional dos outputs e preservado, mas benchmarks de
  performance do programa ficam artificialmente baixos.

Impacto:

- Pode transformar uma execucao de 11 s em uma copia de ficheiros.
- Dificulta comparar otimizacoes reais.

Patch sugerido:

```c
static int cache_desativada(void)
{
    const char *env = getenv("LI3_DISABLE_RESULT_CACHE");
    return env && (*env == '1' || *env == 'y' || *env == 'Y');
}

/* main */
if (!cache_desativada() && restaurar_cache_resultados(pastaDados, ficheiroInput))
    return 0;
...
if (!cache_desativada())
    guardar_cache_resultados(pastaDados, ficheiroInput);
```

Ganho esperado:

- Nao acelera a execucao real, mas torna benchmarks corretos.

### 2. CRITICO - Q2 recalcula ranking por query

Ficheiros:

- `src/queries/querie2.c`
- `src/gestores/gestor_avioes.c`

Atual:

- Para cada Q2:
  - percorre todos os avioes: O(A)
  - cria `GArray`
  - ordena: O(A log A)
  - imprime top N

Com `inputs_fase2.txt`, ha 50 Q2. No grande, A=5000, custo ainda moderado;
em inputs maiores, cresce linearmente por comando.

Otimizado:

- Materializar:
  - ranking global por contagem desc/id asc
  - ranking por fabricante quando necessario
- Query:
  - sem filtro: O(N)
  - com filtro: O(log F + N) ou O(N) sobre array do fabricante

Complexidade:

- Atual: Q2 = O(A log A) por query.
- Otimizada: build = O(A log A), query = O(N).

Patch sugerido:

```c
typedef struct {
    GArray *ranking_global;          /* ContadorVoos ordenado */
    GHashTable *ranking_por_fab;     /* fabricante -> GArray ordenado */
} avioes_q2_index_t;
```

Adicionar em `gestor_avioes` um campo `q2_index` e construir apos carga ou
lazy na primeira Q2. A query passa a iterar sobre o array ja ordenado.

Ganho estimado:

- 2-8% no input atual grande.
- 10-25% se houver muitas Q2.

### 3. CRITICO - Q3 percorre universo 26^3 por query

Ficheiros:

- `src/gestores/gestor_voos.c`
- `src/queries/querie3.c`

Atual:

- `gestor_voos_preparar_q3` cria `q3_contagens[26^3]`, mas so aloca arrays
  para origens observadas.
- `gestor_voos_melhor_origem_intervalo` percorre sempre 17.576 indices.

Complexidade:

- Build: O(V + U * D_observado), onde U=17576 e D=range de dias.
- Query: O(U), mesmo com poucos aeroportos reais.

Otimizado:

- Guardar array compacto `q3_active_origins` com apenas origens que tiveram
  voos.
- Query percorre A_active, nao 26^3.
- Opcional: cache de resultados por `(start_idx,end_idx)`.

Complexidade:

- Query atual: O(17576).
- Query otimizada: O(A_active), tipicamente <= 7354; com cache: O(1) para
  intervalos repetidos.

Patch sugerido:

```c
/* gestor_voos */
int *q3_active_origins;
guint q3_active_len;

/* em q3_visit_voo, quando counts e criado */
g->q3_active_origins[g->q3_active_len++] = orig_idx;

/* em gestor_voos_melhor_origem_intervalo */
for (guint k = 0; k < gestor->q3_active_len; k++) {
    int idx = gestor->q3_active_origins[k];
    int *counts = gestor->q3_contagens[idx];
    ...
}
```

Ganho estimado:

- 1-5% no input atual.
- Alto se Q3 for frequente.

### 4. ALTO IMPACTO - multiplas passagens completas por voos

Ficheiro: `src/gestores/gestor_voos.c`

Atual:

- Durante insercao:
  - calcula min/max Q3.
  - agrega Q5.
- Depois:
  - `gestor_voos_atualizar_contagens_aeroportos`: nova passagem O(V).
  - `gestor_voos_preparar_q3`: nova passagem O(V), mais prefix sums.

Otimizado:

- Ao carregar reservas, ja se incrementam passageiros por voo.
- As contagens de aeroportos dependem do numero final de passageiros, por isso
  precisam ocorrer apos reservas.
- Mas Q3 nao depende de reservas; poderia ser construido incrementalmente
  durante carga de voos usando uma estrutura sparse por origem/dia, evitando uma
  passagem completa posterior.

Complexidade:

- Atual pos-processamento de voos: O(V) + O(V) + prefix.
- Otimizado: remover uma passagem O(V).

Ganho estimado:

- 5-12% no dataset grande.

### 5. ALTO IMPACTO - Q4 cria hash table temporaria por query

Ficheiro: `src/queries/querie4.c`

Atual:

- Sem intervalo: percorre todos top10 semanais.
- Com intervalo: percorre semanas do intervalo.
- Em cada query:
  - cria `GHashTable`
  - aloca `guint` por documento novo
  - itera a hash para achar melhor passageiro

Complexidade:

- Atual: O(W * 10) por query, com alocacoes.
- Otimizacao simples: cache por intervalo `(semana_inicio, semana_fim)`.
- Otimizacao maxima: prefix counts por passageiro ao longo das semanas.

Como W e pequeno, o maior custo e alocacao/hash repetida, nao Big-O.

Patch sugerido:

```c
typedef struct {
    int semana_inicio;
    int semana_fim;
    uint32_t doc_key;
    guint count;
} q4_cache_entry_t;
```

Implementar cache em `gestor_reservas` para intervalos pedidos. Como o input de
teste costuma repetir ranges, isto converte repeticoes em O(1).

Ganho estimado:

- 1-4% no input atual.
- Maior em workloads com muitas Q4.

### 6. ALTO IMPACTO - segunda passagem sobre passageiros para detalhes top10

Ficheiros:

- `src/gestor_programa/gestor_programa.c`
- `src/gestores/gestor_passageiros.c`

Atual:

- Em dataset grande, passageiros sao carregados compactos.
- Depois de calcular top10, le-se `passengers.csv` novamente ate encontrar
  todos documentos top10.

Vantagem:

- Memoria baixa: nao guarda nome/dob de 2M passageiros.

Custo:

- I/O extra e novo parsing parcial de `passengers.csv`.

Alternativas:

- Se objetivo e tempo maximo e memoria pode subir moderadamente:
  carregar `primeiro_nome`, `ultimo_nome`, `dob` para todos.
- Meio termo: guardar offset de linha por documento durante a primeira leitura
  e fazer seeks seletivos para top10 depois.

Complexidade:

- Atual: O(P) extra no pior caso.
- Com detalhes completos: O(1) extra, mais memoria.
- Com offsets: O(P) durante a primeira leitura sem segunda leitura; depois
  O(T) seeks para T documentos top10.

Ganho estimado:

- 3-10% no dataset grande, dependendo da posicao dos documentos top10.
- Memoria extra com detalhes completos: dezenas a centenas de MB.

### 7. MEDIO IMPACTO - parser mmap implementado mas nao usado

Ficheiros:

- `src/parsers/parser.c`
- `src/parsers/parser_reservas.c`

Atual:

- Caminho ativo: `read` com buffer de 8 MiB.
- Caminho `mmap` existe, mas esta `__attribute__((unused))` e
  `parser_mmap_em_uso()` retorna sempre 0.

Recomendacao:

- Testar `mmap` so para `sem_erros` e datasets grandes.
- Expor via `LI3_USE_MMAP=1`.
- Medir: em alguns sistemas `read` sequencial e tao bom quanto `mmap`; nao
  assumir.

Ganho estimado:

- -5% a +8%; depende do kernel/cache page cache.

### 8. MEDIO IMPACTO - parser paralelo serializa a insercao

Ficheiros:

- `src/parsers/parser.c`
- `src/parsers/parser_reservas.c`

Atual:

- Paralelismo so ativa com `LI3_PARSE_UNSAFE=1` e `LI3_PARSE_THREADS`.
- Cada objeto validado chama `adiciona_objeto` sob mutex global.
- Reservas usam mutex em torno de todo `processa_linha`, serializando lookup,
  validacao logica e acumulacoes.

Impacto:

- CPU medido no grande: 105%, confirmando pouca paralelizacao efetiva.

Otimizado:

- Para voos/passengers: shard por hash da chave e fundir no fim.
- Para reservas: processar em threads locais com acumuladores por thread:
  `doc_total_table`, contagens de passageiros por voo, destino por nacionalidade;
  reduzir no final.

Ganho estimado:

- 20-60% em maquinas com 4-8 cores, mas e refatoracao de alto risco.

## EGStores

### Aeroportos

Ficheiro: `src/gestores/gestor_aeroportos.c`

Estrutura:

- `GHashTable *aeroportos`, chave string.
- `aeroporto_t **por_idx` com 26^3 entradas.

Operacoes:

- Lookup por codigo: O(1) direto via indice IATA.
- Iteracao: O(A) via `GHashTable`.

Memoria:

- `por_idx`: 17576 * 8 ~= 137 KiB.
- Hash table duplica chaves com `g_strdup`.

Cache:

- `por_idx` e cache-friendly.
- Entidades sao ponteiros dispersos.

Recomendacao:

- Manter `por_idx`.
- Para performance maxima, remover `GHashTable` e guardar todos aeroportos por
  indice direto. Iteracao pode percorrer `por_idx`.
- Vantagem: menos malloc/chaves GLib.
- Desvantagem: perde flexibilidade para codigos nao IATA.

### Avioes

Ficheiro: `src/gestores/gestor_avioes.c`

Estrutura:

- `GHashTable *tabela` por identificador string.

Operacoes:

- Lookup por id: O(1) medio.
- Q2: O(A log A) por query.

Recomendacao:

- Manter hash para validacao de voos.
- Adicionar `GArray` com ponteiros/records para iteracao compacta e ranking Q2.
- Se ids forem compactaveis, usar open addressing string-interned ou key
  numerica.

### Voos

Ficheiro: `src/gestores/gestor_voos.c`

Estrutura:

- `voo_table`: open addressing `uint64_t key -> uint32_t id`.
- `voo_pool`: blocos contiguos de `voo_t`.
- `voo_t` compacto: dia real, passageiros, atraso, semana, origem, destino,
  status.

Operacoes:

- Lookup por key: O(1) medio.
- Insercao: O(1) medio.
- Iteracao: percorre capacidade da hash, nao apenas count.

Cache:

- `voo_pool` e bom para localidade.
- `voo_table_foreach_id` percorre slots vazios, com load ate 90%.

Recomendacao:

- Baixar load factor para 70-80% para reduzir probing em 4M reservas * ate 2
  voos.
- Manter `uint32_t ids_em_ordem[]` para iteracao O(V) sem percorrer slots vazios.
- Avaliar robin hood hashing para reduzir variancia de probes.

### Passageiros

Ficheiro: `src/gestores/gestor_passageiros.c`

Estrutura:

- `passageiro_table`: open addressing `uint32_t key -> passageiro_t *`.

Operacoes:

- Lookup por documento: O(1) medio.
- Insercao: O(1) medio.

Problemas:

- Load factor 92% e alto para linear probing.
- `capacity` nao e potencia de 2 garantida; usa modulo `%`, mais caro que
  `& mask`.
- Cada passageiro e um malloc separado.

Recomendacao:

- Capacidade potencia de 2 e lookup com `& (capacity - 1)`.
- Load 70-80%.
- Pool/arena de `passageiro_t`.
- Opcional SoA: arrays paralelos `doc_key`, `nacionalidade_id`, `detalhes_id`.

### Reservas

Ficheiro: `src/gestores/gestor_reservas.c`

Estrutura:

- Nao guarda reservas completas.
- `gastos_por_semana`: semana -> `doc_total_table`.
- `top10_por_semana`: semana -> `GPtrArray`.
- `destinos_por_nacionalidade`: nacionalidade -> array `uint32_t[17576]`.
- bitset para IDs de reservas em dataset grande com erros.

Operacoes:

- Insercao reserva: lookups passageiros + voos + acumulacoes.
- Finalizacao top10: percorre todas entradas de cada `doc_total_table`.
- Finalizacao Q6: percorre 17576 destinos por nacionalidade.

Recomendacao:

- Para Q6, trocar array denso por sparse list por nacionalidade:
  - manter `uint32_t counts[17576]` so se numero de nacionalidades for baixo.
  - caso contrario, `GArray` de destinos ativos + hash/array denso temporario.
- Manter top10 incremental por semana durante carga, eliminando `gastos_por_semana`
  completo se so Q4 precisa top10 final. Cuidado: e preciso somar gasto total por
  passageiro antes de saber top10; ainda precisa mapa doc->total por semana.

## Queries

### Q1

Ficheiro: `src/queries/querie1.c`

Atual:

- Parse comando e lookup aeroporto por indice.
- Complexidade: O(1).
- Alocacoes: no gestor de queries ha `g_strdup` da linha.

Otimizado:

- Parsing sem copia; passar token por ponteiro/len.
- Ganho baixo.

### Q2

Atual:

- O(A log A) por query.
- 1 `GArray` por query.
- Scan completo dos avioes.

Otimizado:

- Indice materializado global/por fabricante.
- Query O(N).

### Q3

Atual:

- Query O(17576).
- Sem alocacoes na query.
- Build Q3 tem uma passagem completa por voos.

Otimizado:

- `q3_active_origins` + cache por intervalo.
- Query O(A_active) ou O(1) quando repetida.

### Q4

Atual:

- O(W*10) por query com hash temporaria.
- Aloca `GHashTable` e `guint` por documento contado.

Otimizado:

- Cache de intervalo.
- Prefix materializado por passageiro top10 se houver muitas Q4.

### Q5

Atual:

- Primeira query: constroi/sorta cache O(C log C).
- Depois: O(N).

Otimizado:

- Construir cache no pos-processamento para tirar custo da primeira query.
- Ganho baixo.

### Q6

Atual:

- Lookup hash por nacionalidade O(1).

Otimizado:

- Manter como esta, mas rever memoria de arrays densos por nacionalidade.

## Memoria

Problemas:

- `passageiro_t` e `aviao_t` sao alocados individualmente.
- `GHashTable` duplica chaves em aeroportos/avioes/atrasos.
- `gestor_queries_processar_ficheiro` aloca strings por comando.
- `doc_total_table` faz rehash com `calloc` e percorre tabela inteira.

Sugestoes:

- Arena para `passageiro_t`.
- Pool para `aviao_t` e `aeroporto_t`.
- Intern pool para fabricantes/modelos/airlines, ou guardar IDs.
- Evitar `g_strdup` em parsing de queries.

Exemplo de arena:

```c
typedef struct {
    passageiro_t *blocks[64];
    size_t block_cap;
    size_t count;
} passageiro_pool_t;
```

## Cache e CPU

Bom:

- `voo_t` compacto e armazenado em blocos.
- Chaves numericas para voos e passageiros.
- Aeroportos por indice direto.

Mau:

- Hash tables GLib em hot paths de avioes/aeroportos/strings.
- Load factors altos nas tabelas custom.
- Q4 cria hash temporaria por query.
- Parser paralelo tem mutex grosseiro.

AoS vs SoA:

- `voo_t`: AoS atual e bom porque as queries usam varios campos do mesmo voo
  durante pos-processamento.
- Passageiros: SoA seria melhor para `doc_key`/`nacionalidade`, porque a carga
  e os lookups usam poucos campos.
- Avioes: AoS e aceitavel; adicionar array ordenado melhora iteracao Q2.

## Makefile

Makefile atual ja usa:

- `-O3`
- `-DNDEBUG`
- `-march=native -mtune=native` com `NATIVE=1`
- `-flto=auto` com `LTO=1`

Flags a testar:

- `-O2`: baseline mais estavel, por vezes menor binario/cache i-cache.
- `-O3`: atual, bom para loops.
- `-Ofast`: pode alterar semantica floating-point; risco para outputs `%.3f`.
- `-ffast-math`: nao recomendado sem validar todos outputs.
- `-fomit-frame-pointer`: ganho pequeno; dificulta profiling.
- `-funroll-loops`: pode piorar i-cache; testar.
- `-flto=auto`: manter.
- `-fno-plt`: ganho pequeno em chamadas dinamicas.
- `-pthread`: explicito para compile/link.
- PGO: recomendado.

PGO:

```sh
make clean
make MODE=release CFLAGS_EXTRA="-fprofile-generate" LDFLAGS_EXTRA="-fprofile-generate"
./programa-principal ./sem_erros_grande inputs_fase2.txt
make clean
make MODE=release CFLAGS_EXTRA="-fprofile-use -fprofile-correction" LDFLAGS_EXTRA="-fprofile-use"
```

## Plano de benchmark

Cold run, sem cache de outputs:

```sh
LI3_DISABLE_RESULT_CACHE=1 /usr/bin/time -v ./programa-principal ./sem_erros_grande inputs_fase2.txt
```

Teste funcional:

```sh
LI3_DISABLE_RESULT_CACHE=1 ./programa-testes ./sem_erros inputs_fase2.txt resultados-esperados
LI3_DISABLE_RESULT_CACHE=1 ./programa-testes ./sem_erros_grande inputs_fase2.txt resultados_esperados-grande
```

Hyperfine:

```sh
hyperfine --warmup 1 --runs 5 \
  'LI3_DISABLE_RESULT_CACHE=1 ./programa-principal ./sem_erros inputs_fase2.txt' \
  'LI3_DISABLE_RESULT_CACHE=1 ./programa-principal ./sem_erros_grande inputs_fase2.txt'
```

Perf:

```sh
perf stat -d -r 5 -- env LI3_DISABLE_RESULT_CACHE=1 ./programa-principal ./sem_erros_grande inputs_fase2.txt
perf record -g -- env LI3_DISABLE_RESULT_CACHE=1 ./programa-principal ./sem_erros_grande inputs_fase2.txt
perf report
```

Callgrind:

```sh
valgrind --tool=callgrind --callgrind-out-file=callgrind.out \
  ./programa-principal ./sem_erros inputs_fase2.txt
kcachegrind callgrind.out
```

Cachegrind:

```sh
valgrind --tool=cachegrind --cachegrind-out-file=cachegrind.out \
  ./programa-principal ./sem_erros inputs_fase2.txt
cg_annotate cachegrind.out | head -80
```

Massif:

```sh
valgrind --tool=massif --massif-out-file=massif.out \
  ./programa-principal ./sem_erros inputs_fase2.txt
ms_print massif.out | less
```

Gprof:

```sh
make clean
make MODE=release CFLAGS_EXTRA="-pg" LDFLAGS_EXTRA="-pg"
./programa-principal ./sem_erros inputs_fase2.txt
gprof ./programa-principal gmon.out > gprof.txt
```

## Roadmap

Quick wins:

1. Adicionar `LI3_DISABLE_RESULT_CACHE`.
2. Evitar `g_strdup`/`strtok_r` no parsing de queries.
3. Adicionar `q3_active_origins`.
4. Construir Q5 cache antes de processar queries.
5. Baixar load factor de `passageiro_table` e `voo_table` para 75-80% e usar
   potencia de 2.

Ganhos medios:

1. Q2 materializado global/por fabricante.
2. Cache de Q4 por intervalo.
3. Guardar lista compacta de IDs de voos para iteracoes.
4. Opcao `LI3_USE_MMAP=1` e benchmark A/B.
5. Offsets de passageiros para detalhes top10 sem segunda leitura completa.

Ganhos maximos:

1. Paralelizar reservas com acumuladores por thread e reduce final.
2. Migrar passageiros para SoA + arena.
3. Substituir GLib hash tables em hot paths por open addressing/robin hood.
4. Construir indices Q3 incrementalmente durante carga de voos.
5. PGO como target de release de benchmark.

## Estimativa percentual

- Quick wins: 5-15%.
- Ganhos medios: 15-35%.
- Ganhos maximos: 35-65% em CPU multi-core, com maior uso de memoria.

Estas estimativas assumem que a cache de outputs esta desativada nas medicoes.
