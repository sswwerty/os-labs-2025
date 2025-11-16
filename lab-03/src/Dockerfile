FROM ubuntu:22.04

# Используем эмуляцию x86_64 для Mac ARM, ставим компилятор и strace
RUN dpkg --add-architecture amd64 \
    && apt-get update \
    && apt-get install -y gcc make strace libc6:amd64 libc6-dev:amd64 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY parent.c child.c Makefile ./

RUN make

CMD ["bash"]