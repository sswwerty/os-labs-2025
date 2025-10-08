// parent.cpp
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <cstring>

int main() {
    std::string filename;
    std::cout << "Enter output filename: ";
    std::getline(std::cin, filename);

    if (filename.empty()) {
        std::cerr << "Filename cannot be empty!\n";
        return 1;
    }

    // Создаём два pipe'а
    int pipe1[2]; // parent -> child (stdin)
    int pipe2[2]; // child -> parent (stdout) — опционально

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс

        // Закрываем ненужные концы pipe'ов
        close(pipe1[1]); // закрываем write-end pipe1
        close(pipe2[0]); // закрываем read-end pipe2

        // Перенаправляем stdin из pipe1[0]
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        // Перенаправляем stdout в pipe2[1] (опционально)
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);

        // Подготавливаем аргументы для execv
        char* child_argv[] = {
            const_cast<char*>("./child"),
            const_cast<char*>(filename.c_str()),
            nullptr
        };

        execv("./child", child_argv);
        perror("execv failed");
        exit(1);
    } else {
        // Родительский процесс

        close(pipe1[0]); // не читаем из pipe1
        close(pipe2[1]); // не пишем в pipe2

        std::string input_line;
        std::cout << "Enter lines of floats (Ctrl+D to finish):\n";
        while (std::getline(std::cin, input_line)) {
            if (input_line.empty()) continue;
            input_line += '\n'; // getline убирает \n
            write(pipe1[1], input_line.c_str(), input_line.size());
        }

        // Закрываем write-end: это сигнализирует ребёнку об окончании ввода
        close(pipe1[1]);

        // (Опционально) читаем ответ от ребёнка
        char buffer[256];
        ssize_t bytes;
        std::cout << "Child output (if any):\n";
        while ((bytes = read(pipe2[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes] = '\0';
            std::cout << buffer;
        }
        close(pipe2[0]);

        // Ждём завершения дочернего процесса
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            std::cout << "Child exited with status " << WEXITSTATUS(status) << "\n";
        }
    }

    return 0;
}