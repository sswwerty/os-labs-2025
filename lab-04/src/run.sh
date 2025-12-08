#!/usr/bin/env bash
# Пример запуска контейнера, выполнения strace для static и dynamic и сохранение логов на хост.
IMAGE=lab4_min
CONTAINER=lab4_min_run
# соберём образ
docker build -t $IMAGE .

# запустим контейнер и в нём выполним команды записи strace
docker run --rm -it \
  --cap-add=SYS_PTRACE \
  --security-opt seccomp=unconfined \
  -v "$(pwd)/strace_logs":/work/strace_logs \
  $IMAGE bash -lc "mkdir -p strace_logs && \
    printf '1 0.0 0.001\n2 14 21\nq\n' | strace -ff -o strace_logs/strace_static -tt -s 2000 ./prog && \
    printf '1 0.0 0.001\n2 14 21\n0\n1 0.0 0.001\nq\n' | strace -ff -o strace_logs/strace_dynamic -tt -s 2000 ./prog dynamic ; exit"
