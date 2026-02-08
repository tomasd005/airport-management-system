#!/usr/bin/env python3
import json
import sys
from pathlib import Path

def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)

def fmt(val, digits=3):
    if isinstance(val, (int, float)):
        return f"{val:.{digits}f}" if isinstance(val, float) else str(val)
    return str(val)

def render_table(data):
    qrows = []
    for q in data.get("queries", []):
        qrows.append(
            f"<tr><td>Q{q.get('query')}</td>"
            f"<td>{q.get('corretos')}/{q.get('total')}</td>"
            f"<td>{fmt(q.get('percentagem', 0.0), 2)}%</td>"
            f"<td>{fmt(q.get('tempo_medio_ms', 0.0), 3)}</td>"
            f"<td>{fmt(q.get('tempo_min_ms', 0.0), 3)}</td>"
            f"<td>{fmt(q.get('tempo_max_ms', 0.0), 3)}</td></tr>"
        )
    return "\n".join(qrows)

def render_dataset_card(title, data):
    return f"""
<section class="card">
  <h2>{title}</h2>
  <div class="meta">
    <div><strong>Dataset</strong>: {data.get('dataset','-')}</div>
    <div><strong>Input</strong>: {data.get('input','-')}</div>
    <div><strong>Esperados</strong>: {data.get('esperados','-')}</div>
  </div>
  <div class="summary">
    <div><strong>Tempo total (s)</strong>: {fmt(data.get('tempo_execucao_s', 0.0), 3)}</div>
    <div><strong>Memoria pico (MB)</strong>: {fmt(data.get('memoria_pico_mb', 0.0), 1)}</div>
    <div><strong>Testes OK</strong>: {data.get('testes_ok', 0)}/{data.get('testes_total', 0)}</div>
  </div>
  <table>
    <thead>
      <tr>
        <th>Query</th>
        <th>Corretos/Total</th>
        <th>%</th>
        <th>Tempo medio (ms)</th>
        <th>Tempo min (ms)</th>
        <th>Tempo max (ms)</th>
      </tr>
    </thead>
    <tbody>
      {render_table(data)}
    </tbody>
  </table>
</section>
"""

def main():
    if len(sys.argv) != 4:
        print("Uso: benchmark_summary.py <regular.json> <large.json> <output.html>")
        return 1
    reg_path, large_path, out_path = map(Path, sys.argv[1:])

    if not reg_path.exists() or not large_path.exists():
        print("Faltam ficheiros JSON de benchmark.")
        return 1

    reg = load_json(reg_path)
    large = load_json(large_path)

    html = f"""
<!doctype html>
<html lang="pt">
<head>
  <meta charset="utf-8" />
  <title>Resumo de Benchmarks LI3</title>
  <style>
    body {{ font-family: Arial, sans-serif; margin: 24px; color: #111; }}
    h1 {{ margin-bottom: 8px; }}
    .grid {{ display: grid; grid-template-columns: 1fr; gap: 20px; }}
    .card {{ border: 1px solid #ddd; border-radius: 8px; padding: 16px; }}
    .meta, .summary {{ display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 6px; margin: 8px 0; }}
    table {{ width: 100%; border-collapse: collapse; margin-top: 12px; }}
    th, td {{ border: 1px solid #ddd; padding: 6px 8px; text-align: left; }}
    th {{ background: #f3f3f3; }}
    .note {{ margin-top: 12px; color: #444; font-size: 0.9em; }}
  </style>
</head>
<body>
  <h1>Resumo de Benchmarks</h1>
  <div class="grid">
    {render_dataset_card(reg.get('dataset', 'Dataset A'), reg)}
    {render_dataset_card(large.get('dataset', 'Dataset B'), large)}
  </div>
  <div class="note">Gerado automaticamente a partir de benchmark-regular.json e benchmark-large.json.</div>
</body>
</html>
"""

    out_path.write_text(html, encoding='utf-8')
    print(f"Resumo HTML gerado: {out_path}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
