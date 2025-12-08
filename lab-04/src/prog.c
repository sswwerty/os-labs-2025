#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include "calc.h"

/* Типы */
typedef float (*deriv_fn_t)(float, float);
typedef int   (*gcf_fn_t)(int, int);

#define LINE_SIZE 256

/* Для статического режима — используем функции напрямую (они будут доступны если
   программа линкуется с libimpl1.so при сборке) */

/* В динамическом режиме — загрузка двух реализаций .so, переключение по 0 */
struct dyn_impl {
    void *handle;
    deriv_fn_t deriv;
    gcf_fn_t gcf;
    const char *path;
};

/* Вспомогательная функция: сделаем явный open() и mmap() малого временного файла
   перед dlopen, чтобы в strace точно было open/mmap. (dlopen сам вызывает open/mmap,
   но явный open помогает быстрее найти шаг). */
static void provoke_syscalls(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd >= 0) {
        void *m = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
        if (m != MAP_FAILED) {
            volatile char c = ((char*)m)[0];
            (void)c;
            munmap(m, 4096);
        }
        close(fd);
    }
    void *a = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (a != MAP_FAILED) {
        ((char*)a)[0] = 42;
        munmap(a, 4096);
    }
}

int main(int argc, char **argv) {
    int dynamic_mode = 0;
    if (argc >= 2 && strcmp(argv[1], "dynamic") == 0) dynamic_mode = 1;
    printf("prog (mode: %s)\n", dynamic_mode ? "dynamic" : "static");

    const char *paths[2] = {"./libimpl1.so", "./libimpl2.so"};
    struct dyn_impl libs[2] = {{0}};
    int cur_idx = 0;
    struct dyn_impl *current = NULL;

    if (dynamic_mode) {
        for (int i = 0; i < 2; ++i) {
            provoke_syscalls(paths[i]);
            void *h = dlopen(paths[i], RTLD_LAZY);
            if (!h) {
                fprintf(stderr, "dlopen(%s) failed: %s\n", paths[i], dlerror());
                libs[i].handle = NULL;
            } else {
                libs[i].handle = h;
                libs[i].path = paths[i];
                dlerror();
                libs[i].deriv = (deriv_fn_t)dlsym(h, "Derivative");
                libs[i].gcf   = (gcf_fn_t)dlsym(h, "GCF");
                printf("Loaded %s\n", paths[i]);
            }
        }
        if (libs[0].handle) { current = &libs[0]; cur_idx = 0; }
        else if (libs[1].handle) { current = &libs[1]; cur_idx = 1; }
        else { fprintf(stderr, "No libraries loaded. Exiting.\n"); return 1; }
    } else {
        printf("Static mode: using direct calls (program must be linked to one implementation).\n");
    }

    char line[LINE_SIZE];
    printf("Команды:\n0 - переключить реализацию (только dynamic)\n1 A deltaX  - Derivative(cos)\n2 A B       - GCF\nq or exit - выход\n");

    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        char *p = line;
        while (*p==' '||*p=='\t') ++p;
        if (strncmp(p, "q", 1) == 0 || strncmp(p, "exit", 4) == 0) break;
        if (p[0] == '\n' || p[0] == '\0') continue;
        char cmd;
        if (sscanf(p, " %c", &cmd) != 1) continue;

        if (cmd == '0') {
            if (!dynamic_mode) {
                printf("Команда 0 доступна только в dynamic режиме.\n");
            } else {
                int other = (cur_idx == 0) ? 1 : 0;
                if (libs[other].handle) {
                    current = &libs[other];
                    cur_idx = other;
                    printf("Switched to %s\n", paths[other]);
                } else {
                    printf("Другая библиотека не загружена.\n");
                }
            }
        } else if (cmd == '1') {
            float A, dx;
            if (sscanf(p+1, " %f %f", &A, &dx) != 2) { printf("Неверные аргументы. Формат: 1 A deltaX\n"); continue; }
            float res;
            if (dynamic_mode) {
                if (!current || !current->deriv) { printf("Функция недоступна.\n"); continue; }
                res = current->deriv(A, dx);
            } else {
                res = Derivative(A, dx);
            }
            if (isnan(res)) printf("Ошибка: deltaX == 0\n");
            else printf("Derivative(cos) at %.6f with dx=%.6f -> %.10f\n", A, dx, res);
        } else if (cmd == '2') {
            int A, B;
            if (sscanf(p+1, " %d %d", &A, &B) != 2) { printf("Неверные аргументы. Формат: 2 A B\n"); continue; }
            int g;
            if (dynamic_mode) {
                if (!current || !current->gcf) { printf("Функция недоступна.\n"); continue; }
                g = current->gcf(A, B);
            } else {
                g = GCF(A, B);
            }
            printf("GCF(%d, %d) = %d\n", A, B, g);
        } else {
            printf("Неизвестная команда.\n");
        }
    }

    if (dynamic_mode) {
        for (int i = 0; i < 2; ++i) {
            if (libs[i].handle) dlclose(libs[i].handle);
        }
    }

    printf("bye\n");
    return 0;
}