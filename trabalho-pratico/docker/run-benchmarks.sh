#!/usr/bin/env bash
set -euo pipefail

cd /app

make programa-principal programa-testes

mkdir -p resultados

./programa-testes sem_erros inputs_fase2.txt resultados-esperados
cp resultados/benchmark.json resultados/benchmark-regular.json
cp resultados/benchmark.html resultados/benchmark-regular.html

./programa-testes sem_erros_grande inputs_fase2.txt resultados_esperados-grande
cp resultados/benchmark.json resultados/benchmark-large.json
cp resultados/benchmark.html resultados/benchmark-large.html

python3 docker/benchmark_summary.py resultados/benchmark-regular.json resultados/benchmark-large.json resultados/benchmark-summary.html

echo "Benchmarks completos."
echo "Regular: resultados/benchmark-regular.json e resultados/benchmark-regular.html"
echo "Large:   resultados/benchmark-large.json e resultados/benchmark-large.html"
echo "Resumo:  resultados/benchmark-summary.html"
