#!/bin/bash
# Uso: ./bench.sh <nome_da_plataforma>
# Ex.: ./bench.sh notebook   ->  gera tempos_notebook.csv
# Rode uma vez em cada computador. Antes, confira a carga com `top`.

PLAT=${1:-maquina}
OUT="tempos_${PLAT}.csv"
REPS=5

gcc -O2 -o pi_threads pi_threads.c -lm -lpthread || exit 1

echo "plataforma,threads,execucao,tempo_s" > "$OUT"
TIMEFORMAT=%R   # imprime apenas o tempo real (wall-clock) em segundos

for n in 1 2 4 8 16 32; do
  for r in $(seq 1 $REPS); do
    t=$( { time ./pi_threads $n > /dev/null; } 2>&1 )
    echo "$PLAT,$n,$r,${t/,/.}" >> "$OUT"
    echo "threads=$n execucao=$r tempo=${t}s"
  done
done

echo "Resultados em $OUT"
