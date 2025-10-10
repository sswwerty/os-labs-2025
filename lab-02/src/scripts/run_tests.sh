#!/usr/bin/env bash
# Запускает серию замеров и записывает results.csv в корне проекта (lab-02/results.csv)
set -e

# ROOT — корень проекта lab-02 (один уровень выше папки scripts)
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT"          # теперь src — текущая папка, т.е. здесь лежит qsort_mt и main.c
OUT="$ROOT/results.csv"
PLOTS="$ROOT/plots"

mkdir -p "$PLOTS"
echo "N,max_threads,seed,elapsed,sorted" > "$OUT"

# конфигурация тестов (изменяй при необходимости)
Ns=(100000 200000 500000 1000000)
Threads=(1 2 4 8)
REPEATS=3

echo "Starting tests. Results -> $OUT"

# удаляем строку cd "$SRC", так как бинарь уже находится здесь
for N in "${Ns[@]}"; do
  for m in "${Threads[@]}"; do
    for r in $(seq 1 $REPEATS); do
      seed=$RANDOM
      echo "Run: N=$N threads=$m repeat=$r seed=$seed"
      ./qsort_mt -n $N -m $m -r $seed -o "$OUT"
      sleep 0.2
    done
  done
done

echo "All tests finished. CSV: $OUT"