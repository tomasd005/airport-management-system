Guiao de Defesa Oral - LI3 (Fase 2)

Objetivo
- Explicar arquitetura, estruturas de dados e escolhas de implementacao.
- Mostrar como as queries sao resolvidas e justificadas.
- Estar preparado para perguntas do porquê das decisoes.
- Ter um mapa claro de onde esta cada coisa no codigo.


1) Introducao (1-2 min)
- Projeto em C para gerir aeroportos, avioes, voos, passageiros e reservas.
- Carregamos CSVs, validamos, guardamos em estruturas eficientes e respondemos a queries.
- Foco da fase 2: modularidade, encapsulamento e pre-processamento para acelerar queries.

Mensagem-chave: tirar trabalho das queries e colocar no carregamento.


2) Como compila e o que corre (fluxo de ponta a ponta)

Compilacao (Makefile)
- O comando `make` chama o GCC e gera 3 executaveis:
  - programa-principal
  - programa-testes
  - programa-interativo
- Isso e feito no ficheiro `Makefile` na raiz.
- Os ficheiros .c compilados estao em `src/`.

Execucao normal
1) Corres o programa principal:
   - `./programa-principal <pasta_datasets> <ficheiro_input>`
2) O `src/programas/main.c` chama `gestor_programa_executa`.
3) O gestor do programa (`src/gestor_programa/gestor_programa.c`):
   - carrega CSVs (airports, aircrafts, flights, passengers, reservations)
   - prepara caches e estruturas
   - depois cria o gestor de queries e processa o ficheiro de comandos
4) Para cada linha do input, o gestor de queries (`src/gestor_programa/gestor_queries.c`):
   - identifica qual query e
   - chama a funcao da query (Q1..Q6)
   - escreve o output no ficheiro `resultados/commandX_output.txt`

Programa-testes
- `src/programas/teste.c` chama `gestor_testes_executar`.
- Ele corre o programa-principal, depois compara com os outputs esperados.

Programa-interativo
- `src/programas/interativo.c` usa `gestor_interativo`.
- Permite ler comandos no terminal, executar queries e mostrar resultados.


3) Arquitetura (2-3 min)
- Fluxo geral: Parser/Validacoes -> Gestores -> Queries -> Output.
- Gestor de Programa coordena carregamento e execucao.
- Gestor de Queries interpreta comandos e chama a query certa.
- Utils partilhadas para parsing, datas e chaves numericas.

Porque esta arquitetura?
- Separacao de responsabilidades, mais facil de manter e testar.
- Cada modulo tem fronteiras claras e reduz acoplamento.


4) Onde esta cada modulo (explicado de forma simples)

Programas (entrada)
- `src/programas/main.c` -> programa principal (modo batch)
- `src/programas/teste.c` -> programa de testes automaticos
- `src/programas/interativo.c` -> modo interativo

Gestor do programa
- `src/gestor_programa/gestor_programa.c`
  - cria gestores
  - carrega dados
  - inicia processamento de queries
- `src/gestor_programa/gestor_queries.c`
  - interpreta o ficheiro de input
  - chama query1..query6

Gestores (dados e caches)
- `src/gestores/gestor_aeroportos.c`
  - tabela de aeroportos
  - funcoes para inserir e obter aeroportos
- `src/gestores/gestor_avioes.c`
  - tabela de avioes
  - contagem de voos por aviao
- `src/gestores/gestor_voos.c`
  - tabela de voos
  - atrasos por companhia (Q5)
  - prefix sums por dia (Q3)
- `src/gestores/gestor_passageiros.c`
  - tabela de passageiros
- `src/gestores/gestor_reservas.c`
  - tabela de reservas
  - gastos semanais (Q4)
  - destinos por nacionalidade (Q6)

Entidades (structs)
- `src/entidades/aeroportos.c` -> dados do aeroporto
- `src/entidades/avioes.c` -> dados do aviao
- `src/entidades/voos.c` -> dados do voo
- `src/entidades/passageiros.c` -> dados do passageiro
- `src/entidades/reservas.c` -> dados da reserva

Parsing/validacao
- `src/parsers/parser.c` -> separa CSV em colunas
- `src/validacoes/validacao_*.c` -> regras de validacao por entidade

Queries
- `src/queries/querie1.c` ate `src/queries/querie6.c`
  - cada ficheiro corresponde a uma query

Utils
- `src/utils.c` -> datas, trim, helpers


5) Estruturas de dados (2-3 min)
- Hash tables (GLib) para acesso medio O(1) por ID:
  - aeroportos (codigo IATA)
  - avioes (identificador)
  - voos (flight id)
  - passageiros (document number)
- Arrays para contagens densas e prefixos:
  - contagens por dia (Q3)
  - destinos por nacionalidade (Q6)
- Interning de strings (voos) para reduzir duplicacao de origem/destino.

Justificacao:
- Hash table quando IDs sao esparsos.
- Arrays quando a chave e densa (indices numericos).


6) Queries (resumo com justificacao)
Q1 - Resumo de aeroporto
- Chegadas/partidas pre-computadas a partir das reservas.
- Query apenas consulta contadores e imprime.

Q2 - Top N avioes com mais voos
- Cada aviao guarda contagem incrementada na carga.
- Query filtra/ordena e imprime top N.

Q3 - Aeroporto com mais partidas num periodo
- Prefix sums por dia e origem.
- Query usa diferencas acumuladas, sem percorrer todos os voos.

Q4 - Passageiro mais vezes no top10 semanal de gastos
- Durante carga: gastos agregados por semana/passageiro.
- Finalizacao: top10 por semana guardado.
- Query apenas conta presencas no intervalo.

Q5 - Top N companhias com maior atraso medio
- Durante carga: total atraso + num voos por companhia.
- Query gera lista ordenada (cache) e reutiliza.

Q6 - Destino mais comum por nacionalidade
- Durante carga: contagem de destinos por nacionalidade.
- Guarda-se o destino mais frequente para resposta O(1).


7) Desempenho e otimizações
- Pre-processamento desloca custo para o carregamento.
- Queries ficam leves e consistentes.
- Uso de caches e agregacoes evita percorrer datasets repetidamente.

Trade-off:
- Mais memoria e tempo de carregamento em troca de queries rapidas.


8) Encapsulamento e modularidade
- Estruturas opacas: acesso apenas por interface publica.
- Gestores controlam vida dos dados.
- Alteracoes internas nao afetam o resto do sistema.


9) Testes
- Programa-testes compara outputs automaticamente.
- Validacao com datasets sem_erros e com_erros.
- Valgrind usado para leaks e acessos invalidos.


10) Perguntas provaveis (respostas mais detalhadas)

Q: Onde e que carrega os dados? E como?
A: No `src/gestor_programa/gestor_programa.c`, na funcao `gestor_programa_executa`.
   La chama `gestor_aeroportos_carregar`, `gestor_avioes_carregar`, `gestor_voos_carregar_com_validacao`,
   `gestor_passageiros_carregar` e `gestor_reservas_carregar_com_validacao`.
   Cada uma dessas funcoes usa o parser para ler o CSV linha a linha.

Q: Onde estao as regras de validacao?
A: Em `src/validacoes/validacao_*.c` (por entidade). Ex:
   - voos: `validacao_voos.c`
   - passageiros: `validacao_passageiros.c`
   - reservas: `validacao_reservas.c`

Q: Onde e que as queries sao chamadas?
A: Em `src/gestor_programa/gestor_queries.c`.
   Esse ficheiro le o input, identifica a query e chama `query1`, `query2`, ...

Q: Onde esta o codigo da query X?
A: Em `src/queries/querieX.c`.
   Por exemplo, Q3 esta em `src/queries/querie3.c`.

Q: Onde se guardam os dados em memoria?
A: Em `src/gestores/*`.
   Cada gestor tem uma tabela principal (hash table) e, se preciso, estruturas auxiliares.

Q: Porque usar GLib e hash tables?
A: IDs sao strings e esparsos. Hash table da acesso O(1) medio.
   GLib reduz bugs e evita reimplementar estruturas.

Q: Como calculam semanas?
A: Em `src/utils.c`. Convertem a data para dias desde uma epoca e depois para numero de semana.

Q: Como evitam percorrer o dataset a cada query?
A: Pre-processamento no carregamento.
   Ex: Q3 guarda prefix sums, Q4 guarda top10 semanal, Q6 guarda destino mais comum por nacionalidade.

Q: Como sao gerados os outputs?
A: Cada query escreve num ficheiro `resultados/commandX_output.txt`.
   O nome vem do numero da linha no input.

Q: Como funciona o programa-testes?
A: `src/programas/teste.c` corre o programa principal e compara
   `resultados/commandX_output.txt` com `resultados_esperados/...`.

Q: O que foi mais dificil?
A: Equilibrar memoria e tempo no dataset grande.

Q: Melhorias futuras?
A: Profiling mais fino e possivel paralelizacao do parsing.


11) Fecho (30s)
- Sistema modular, eficiente e testado.
- Principais ganhos: pre-processamento e estruturas adequadas.
- Queries rapidas e resultados corretos.
