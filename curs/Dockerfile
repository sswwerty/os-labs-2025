FROM ubuntu:22.04

# Устанавливаем необходимые инструменты
RUN apt-get update && \
    apt-get install -y gcc make strace && \
    rm -rf /var/lib/apt/lists/*

# Устанавливаем рабочую директорию
WORKDIR /app

# Копируем все исходники в контейнер
COPY . /app

# Компилируем сервер и клиент используя Makefile
RUN make clean && make
