#!/usr/bin/env python3
import csv
from datetime import datetime
from collections import defaultdict

FLIGHTS='/home/cliff/Desktop/LI3/trabalho-pratico/2526-G82/trabalho-pratico/com_erros/flights.csv'

def parse_dt(s):
    if s == 'N/A' or s.strip() == '':
        return None
    for fmt in ('%Y-%m-%d %H:%M', '%Y/%m/%d %H:%M'):
        try:
            return datetime.strptime(s, fmt)
        except Exception:
            pass
    return None

stats = defaultdict(lambda: {'count':0, 'total':0.0})

with open(FLIGHTS, newline='', encoding='utf-8') as f:
    reader = csv.reader(f)
    headers = next(reader)
    for row in reader:
        # flight fields based on parser: id, departure, actual_departure, arrival, actual_arrival, gate, status, origin, dest, aircraft, airline, url
        if len(row) < 12:
            continue
        flight_id = row[0].strip()
        departure = row[1].strip()
        actual_dep = row[2].strip()
        status = row[6].strip()
        airline = row[10].strip()

        if status != 'Delayed':
            continue
        ad = parse_dt(actual_dep)
        d = parse_dt(departure)
        if not ad or not d:
            continue
        # ignore if actual < scheduled
        if ad < d:
            continue
        delay = (ad - d).total_seconds()/60.0
        if delay <= 0.5:
            continue
        stats[airline]['count'] += 1
        stats[airline]['total'] += delay

# compute avg and sort desc
res = []
for airline, v in stats.items():
    avg = v['total']/v['count']
    res.append((airline, v['count'], avg))
res.sort(key=lambda x: (-x[2], x[0]))

out = '/home/cliff/Desktop/LI3/trabalho-pratico/2526-G82/trabalho-pratico/resultados/q5_python_ref.txt'
with open(out,'w',encoding='utf-8') as f:
    for airline,count,avg in res:
        f.write(f"{airline}={count}={avg:.3f}\n")
print('Wrote', out)
