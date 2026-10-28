#!/bin/bash
# Uso: ./bench.sh <nome_da_plataforma>
# Ex.: ./bench.sh notebook   ->  gera ../tempos-csv/tempos_notebook.csv
# Rode uma vez em cada computador. Antes, confira a carga com `top`.

PLAT=${1:-maquina}
REPS=5
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
PROJECT_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
SOURCE_DIR="$PROJECT_DIR/source-code"
OUTPUT_DIR="$PROJECT_DIR/tempos-csv"
EXECUTABLE="$SOURCE_DIR/pi_threads"
OUT="$OUTPUT_DIR/tempos_${PLAT}.csv"

mkdir -p "$OUTPUT_DIR"
gcc -O2 -o "$EXECUTABLE" "$SOURCE_DIR/pi_threads.c" -lm -lpthread || exit 1

echo "plataforma,threads,execucao,tempo_s" > "$OUT"
TIMEFORMAT=%R   # imprime apenas o tempo real (wall-clock) em segundos

for n in 1 2 4 8 16 32; do
  for r in $(seq 1 $REPS); do
    t=$( { time "$EXECUTABLE" "$n" > /dev/null; } 2>&1 )
    echo "$PLAT,$n,$r,${t/,/.}" >> "$OUT"
    echo "threads=$n execucao=$r tempo=${t}s"
  done
done

echo "Resultados em $OUT"
