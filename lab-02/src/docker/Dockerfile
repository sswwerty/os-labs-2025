FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    build-essential \
    strace \
    python3 \
    python3-pip \
    python3-pandas \
    python3-matplotlib \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
COPY .. /work
