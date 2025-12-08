FROM debian:12-slim

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential gcc make ca-certificates strace procps libc6-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /work

# добавляем файлы проекта (при docker build .)
COPY . /work

# собираем
RUN make

# по умолчанию запускаем bash (контейнер интерактивный)
ENTRYPOINT ["/bin/bash"]
