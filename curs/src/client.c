#include "client.h"
#include "protocol.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
void run_client(void) {
    char fifo[64];
    sprintf(fifo, "/tmp/bc_client_%d", getpid());
    mkfifo(fifo, 0666);
    int server = open(SERVER_FIFO, O_WRONLY);
    int current_game_id = -1;
    while (1) {
        printf("\n1. Создать игру\n");
        printf("2. Присоединиться по имени\n");
        printf("3. Найти игру\n");
        printf("4. Сделать ход\n");
        printf("5. Выйти\n> ");
        int choice;
        scanf("%d", &choice);
        char msg[128];
        int in;
        if (choice == 1) {
            char name[64];
            int players;
            printf("Имя игры: ");
            scanf("%s", name);
            printf("Кол-во игроков: ");
            scanf("%d", &players);
            sprintf(msg, "CREATE|%s|%s|%d", fifo, name, players);
            write(server, msg, strlen(msg));
        }
        else if (choice == 2) {
            char name[64];
            printf("Имя игры: ");
            scanf("%s", name);
            sprintf(msg, "JOIN|%s|%s", fifo, name);
            write(server, msg, strlen(msg));
        }
        else if (choice == 3) {
            sprintf(msg, "FIND|%s|", fifo);
            write(server, msg, strlen(msg));
        }
        else if (choice == 4) {
            if (current_game_id == -1) {
                printf("Сначала войдите в игру\n");
                continue;
            }
            char guess[16];
            printf("Введите число: ");
            scanf("%s", guess);
            sprintf(msg, "GUESS|%s|%d|%s", fifo, current_game_id, guess);
            write(server, msg, strlen(msg));
        }
        else if (choice == 5) {
            sprintf(msg, "EXIT|%s|", fifo);
            write(server, msg, strlen(msg));
            unlink(fifo);
            break;
        }
        in = open(fifo, O_RDONLY);
        char resp[128];
        int r = read(in, resp, sizeof(resp) - 1);
        if (r > 0) {
            resp[r] = '\0';
            printf("Сервер: %s\n", resp);
            char *id_pos = strstr(resp, "ID:");
            if (id_pos)
                sscanf(id_pos, "ID: %d", &current_game_id);
        }
        close(in);
    }
}