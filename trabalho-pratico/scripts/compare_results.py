#!/usr/bin/env python3
import os
import sys

def read_lines(path):
    try:
        with open(path, 'r', encoding='utf-8') as f:
            return f.read().splitlines()
    except Exception as e:
        return None

base = os.path.dirname(__file__)
res_dir = os.path.join(base, '..', 'resultados')
exp_dir = os.path.join(base, '..', 'resultados-esperados')
input_file = os.path.join(base, '..', 'inputs_fase2.txt')

# count commands
cmds = []
with open(input_file, 'r', encoding='utf-8') as f:
    for line in f:
        if line.strip():
            cmds.append(line.strip())

mismatches = []
missing = []
for i in range(1, len(cmds)+1):
    res_path = os.path.join(res_dir, f'command{i}_output.txt')
    exp_path = os.path.join(exp_dir, f'command{i}_output.txt')
    r = read_lines(res_path)
    e = read_lines(exp_path)
    if r is None:
        missing.append((i, 'result'))
        continue
    if e is None:
        missing.append((i, 'expected'))
        continue
    if r != e:
        # find first differing line
        ln = 1
        for a, b in zip(r, e):
            if a != b:
                mismatches.append((i, ln, a, b))
                break
            ln += 1
        else:
            # length different
            mismatches.append((i, ln, '\n'.join(r[ln-1:]), '\n'.join(e[ln-1:])))

print(f'Total commands: {len(cmds)}')
print(f'Mismatches: {len(mismatches)}')
print(f'Missing files: {len(missing)}')
print('')
for m in mismatches[:200]:
    i, ln, a, b = m
    print(f'Command {i}: difference at line {ln}')
    print('  Got   :', a)
    print('  Expect:', b)
    print('')
for mm in missing:
    print('Missing:', mm)

# gravar resumo em JSON para leitura programática
try:
    import json
    summary = {
        'total_commands': len(cmds),
        'mismatches': [ {'command': m[0], 'line': m[1], 'got': m[2], 'expected': m[3]} for m in mismatches ],
        'missing': [ {'command': mm[0], 'which': mm[1]} for mm in missing ]
    }
    out_path = os.path.join(base, '..', 'compare_summary.json')
    with open(out_path, 'w', encoding='utf-8') as f:
        json.dump(summary, f, indent=2, ensure_ascii=False)
    print('\nResumo gravado em', out_path)
except Exception:
    pass

sys.exit(0)
