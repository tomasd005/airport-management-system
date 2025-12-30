#!/usr/bin/env python3
"""
Script de diagnóstico para identificar padrões nas diferenças entre
resultados obtidos e esperados para as queries Q2, Q3 e Q5.
"""

import os
import sys
import re
from collections import defaultdict

RESULTADOS_DIR = "resultados"
ESPERADOS_DIR = "resultados-esperados"
INPUTS_FILE = "inputs_fase2.txt"

def detectar_tipo_query(comando_num):
    """Detecta o tipo de query a partir do ficheiro de inputs."""
    with open(INPUTS_FILE, 'r') as f:
        linhas = f.readlines()
        if comando_num <= len(linhas):
            linha = linhas[comando_num - 1].strip()
            if linha:
                partes = linha.split()
                if partes:
                    # Extrair apenas os dígitos do início (ex: "1S" -> 1)
                    tipo_str = partes[0]
                    match = re.match(r'(\d+)', tipo_str)
                    if match:
                        return int(match.group(1))
    return None

def analisar_q2(comando_num):
    """Analisa diferenças na Q2 (contagem de voos por avião)."""
    obtido = f"{RESULTADOS_DIR}/command{comando_num}_output.txt"
    esperado = f"{ESPERADOS_DIR}/command{comando_num}_output.txt"
    
    if not os.path.exists(obtido) or not os.path.exists(esperado):
        return None
    
    # Ler resultados
    obtidos = {}
    with open(obtido, 'r') as f:
        for linha in f:
            linha = linha.strip()
            if linha and ';' in linha:
                partes = linha.split(';')
                if len(partes) >= 4:
                    aviao_id = partes[0]
                    count = int(partes[3])
                    obtidos[aviao_id] = count
    
    esperados = {}
    with open(esperado, 'r') as f:
        for linha in f:
            linha = linha.strip()
            if linha and ';' in linha:
                partes = linha.split(';')
                if len(partes) >= 4:
                    aviao_id = partes[0]
                    count = int(partes[3])
                    esperados[aviao_id] = count
    
    # Encontrar diferenças
    todos_avioes = set(obtidos.keys()) | set(esperados.keys())
    diferencas = []
    
    for aviao in todos_avioes:
        obt = obtidos.get(aviao, 0)
        esp = esperados.get(aviao, 0)
        if obt != esp:
            diferencas.append({
                'aviao': aviao,
                'obtido': obt,
                'esperado': esp,
                'diferenca': obt - esp
            })
    
    return diferencas

def analisar_q3(comando_num):
    """Analisa diferenças na Q3 (aeroporto com mais partidas)."""
    obtido = f"{RESULTADOS_DIR}/command{comando_num}_output.txt"
    esperado = f"{ESPERADOS_DIR}/command{comando_num}_output.txt"
    
    if not os.path.exists(obtido) or not os.path.exists(esperado):
        return None
    
    with open(obtido, 'r') as f:
        linha_obt = f.read().strip()
    
    with open(esperado, 'r') as f:
        linha_esp = f.read().strip()
    
    if linha_obt == linha_esp:
        return None
    
    # Extrair código do aeroporto e contagem
    obt_parts = linha_obt.split(';')
    esp_parts = linha_esp.split(';')
    
    return {
        'obtido': linha_obt,
        'esperado': linha_esp,
        'obtido_aeroporto': obt_parts[0] if len(obt_parts) > 0 else '',
        'esperado_aeroporto': esp_parts[0] if len(esp_parts) > 0 else '',
        'obtido_count': obt_parts[-1] if len(obt_parts) > 0 else '',
        'esperado_count': esp_parts[-1] if len(esp_parts) > 0 else ''
    }

def analisar_q5(comando_num):
    """Analisa diferenças na Q5 (companhias com maior atraso médio)."""
    obtido = f"{RESULTADOS_DIR}/command{comando_num}_output.txt"
    esperado = f"{ESPERADOS_DIR}/command{comando_num}_output.txt"
    
    if not os.path.exists(obtido) or not os.path.exists(esperado):
        return None
    
    # Ler resultados
    obtidos = {}
    with open(obtido, 'r') as f:
        for linha in f:
            linha = linha.strip()
            if linha and '=' in linha:
                partes = linha.split('=')
                if len(partes) >= 3:
                    airline = partes[0]
                    count = int(partes[1])
                    avg_delay = float(partes[2])
                    obtidos[airline] = {'count': count, 'avg': avg_delay}
    
    esperados = {}
    with open(esperado, 'r') as f:
        for linha in f:
            linha = linha.strip()
            if linha and '=' in linha:
                partes = linha.split('=')
                if len(partes) >= 3:
                    airline = partes[0]
                    count = int(partes[1])
                    avg_delay = float(partes[2])
                    esperados[airline] = {'count': count, 'avg': avg_delay}
    
    # Encontrar diferenças
    todos_airlines = set(obtidos.keys()) | set(esperados.keys())
    diferencas = []
    
    for airline in todos_airlines:
        obt = obtidos.get(airline)
        esp = esperados.get(airline)
        
        if obt is None:
            diferencas.append({
                'airline': airline,
                'tipo': 'faltando_obtido',
                'esperado': esp
            })
        elif esp is None:
            diferencas.append({
                'airline': airline,
                'tipo': 'faltando_esperado',
                'obtido': obt
            })
        elif obt['count'] != esp['count'] or abs(obt['avg'] - esp['avg']) > 0.001:
            diferencas.append({
                'airline': airline,
                'tipo': 'diferenca',
                'obtido': obt,
                'esperado': esp,
                'diff_count': obt['count'] - esp['count'],
                'diff_avg': obt['avg'] - esp['avg']
            })
    
    return diferencas

def main():
    print("=" * 60)
    print("DIAGNÓSTICO DE QUERIES - Análise de Padrões")
    print("=" * 60)
    print()
    
    # Ler inputs para determinar quantos comandos há
    with open(INPUTS_FILE, 'r') as f:
        total_comandos = len([l for l in f if l.strip()])
    
    print(f"Total de comandos a processar: {total_comandos}\n")
    
    # Agrupar por tipo de query
    falhas_q2 = []
    falhas_q3 = []
    falhas_q5 = []
    
    for i in range(1, total_comandos + 1):
        tipo = detectar_tipo_query(i)
        if tipo == 2:
            diff = analisar_q2(i)
            if diff:
                falhas_q2.append((i, diff))
        elif tipo == 3:
            diff = analisar_q3(i)
            if diff:
                falhas_q3.append((i, diff))
        elif tipo == 5:
            diff = analisar_q5(i)
            if diff:
                falhas_q5.append((i, diff))
    
    # Relatório Q2
    if falhas_q2:
        print(f"\n{'='*60}")
        print(f"Q2: {len(falhas_q2)} falhas encontradas")
        print(f"{'='*60}\n")
        
        # Padrões
        diferencas_totais = defaultdict(int)
        avioes_com_problemas = defaultdict(int)
        
        for comando, diffs in falhas_q2[:5]:  # Primeiros 5
            print(f"Command {comando}:")
            for diff in diffs[:3]:  # Primeiros 3 aviões
                print(f"  {diff['aviao']}: obtido={diff['obtido']}, esperado={diff['esperado']}, diff={diff['diferenca']}")
                diferencas_totais[diff['diferenca']] += 1
                avioes_com_problemas[diff['aviao']] += 1
            print()
        
        print(f"\nPadrões encontrados:")
        print(f"  Diferenças mais comuns: {dict(diferencas_totais)}")
        print(f"  Aviões com mais problemas: {dict(sorted(avioes_com_problemas.items(), key=lambda x: x[1], reverse=True)[:5])}")
    
    # Relatório Q3
    if falhas_q3:
        print(f"\n{'='*60}")
        print(f"Q3: {len(falhas_q3)} falhas encontradas")
        print(f"{'='*60}\n")
        
        aeroportos_obtidos = defaultdict(int)
        aeroportos_esperados = defaultdict(int)
        
        for comando, diff in falhas_q3:
            print(f"Command {comando}:")
            print(f"  Obtido: {diff['obtido']}")
            print(f"  Esperado: {diff['esperado']}")
            print()
            aeroportos_obtidos[diff['obtido_aeroporto']] += 1
            aeroportos_esperados[diff['esperado_aeroporto']] += 1
        
        print(f"\nPadrões encontrados:")
        print(f"  Aeroportos obtidos: {dict(aeroportos_obtidos)}")
        print(f"  Aeroportos esperados: {dict(aeroportos_esperados)}")
    
    # Relatório Q5
    if falhas_q5:
        print(f"\n{'='*60}")
        print(f"Q5: {len(falhas_q5)} falhas encontradas")
        print(f"{'='*60}\n")
        
        # Estatísticas
        total_diff_count = 0
        total_diff_avg = 0.0
        airlines_com_problemas = defaultdict(list)
        
        for comando, diffs in falhas_q5[:5]:  # Primeiros 5
            print(f"Command {comando}:")
            for diff in diffs[:3]:  # Primeiros 3 airlines
                if diff['tipo'] == 'diferenca':
                    print(f"  {diff['airline']}: count_diff={diff['diff_count']}, avg_diff={diff['diff_avg']:.6f}")
                    total_diff_count += diff['diff_count']
                    total_diff_avg += diff['diff_avg']
                    airlines_com_problemas[diff['airline']].append(diff)
                elif diff['tipo'] == 'faltando_obtido':
                    print(f"  {diff['airline']}: FALTANDO no obtido")
                elif diff['tipo'] == 'faltando_esperado':
                    print(f"  {diff['airline']}: FALTANDO no esperado")
            print()
        
        if total_diff_count != 0:
            print(f"\nPadrões encontrados:")
            print(f"  Diferença total de voos: {total_diff_count}")
            print(f"  Diferença média de delay: {total_diff_avg/len(falhas_q5):.6f}")
            print(f"  Airlines com mais problemas: {list(airlines_com_problemas.keys())[:5]}")
    
    print(f"\n{'='*60}")
    print("RESUMO")
    print(f"{'='*60}")
    print(f"Q2: {len(falhas_q2)}/{50} falhas")
    print(f"Q3: {len(falhas_q3)}/{50} falhas")
    print(f"Q5: {len(falhas_q5)}/{50} falhas")

if __name__ == "__main__":
    main()
