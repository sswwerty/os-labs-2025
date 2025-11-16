'''
# Собрать образ для amd64
docker buildx build --platform linux/amd64 -t os-lab3 .

# Запустить контейнер с текущей папкой
docker run --rm -it -v "$(pwd)":/app -w /app os-lab3 bash

# В контейнере собрать (если нужно)
make clean
make

# Проверка версий
gcc --version
make --version
strace --version

# Запуск с трассировкой
strace -f -o trace.log ./parent

# Вывести только сигналы
strace -e trace=signal -f ./parent 
'''