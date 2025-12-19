#include "game.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
game_t games[10];
int game_count = 0;
void run_server(void) {
    mkfifo(SERVER_FIFO, 0666);
    int fd = open(SERVER_FIFO, O_RDONLY);
    char buffer[MAX_MSG];
    printf("Сервер запущен\n");
    while (1) {
        int r = read(fd, buffer, sizeof(buffer) - 1);
        if (r <= 0) continue;

        buffer[r] = '\0';
        char cmd[32];
        char client_fifo[64];
        char data[128];
        memset(cmd, 0, sizeof(cmd));
        memset(client_fifo, 0, sizeof(client_fifo));
        memset(data, 0, sizeof(data));
        char *p = buffer;
        char *cmd_end = strchr(p, '|');
        if (!cmd_end) continue;
        strncpy(cmd, p, cmd_end - p);
        p = cmd_end + 1;
        char *fifo_end = strchr(p, '|');
        if (!fifo_end) continue;
        strncpy(client_fifo, p, fifo_end - p);
        p = fifo_end + 1;
        strncpy(data, p, sizeof(data) - 1);
        int out = open(client_fifo, O_WRONLY);
        char reply[128];
        memset(reply, 0, sizeof(reply));
        if (strcmp(cmd, "CREATE") == 0) {
            char game_name[64];
            int players;
            char *sep = strchr(data, '|');
            if (!sep) {
                sprintf(reply, "Ошибка формата CREATE");
                write(out, reply, strlen(reply));
                close(out);
                continue;
            }
            strncpy(game_name, data, sep - data);
            game_name[sep - data] = '\0';
            players = atoi(sep + 1);
            int exists = 0;
            for (int i = 0; i < game_count; i++) {
                if (!games[i].finished && strcmp(games[i].name, game_name) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (exists) {
                sprintf(reply, "Игра с таким именем уже существует");
            } else {
                game_t g;
                g.id = ++game_count;
                strcpy(g.name, game_name);
                g.players_needed = players;
                g.players_connected = 1;
                g.finished = 0;
                generate_secret(g.secret);
                games[game_count - 1] = g;
                sprintf(reply,
                        "Игра '%s' создана. ID: %d. Ожидаем %d игрока(ей)",
                        g.name, g.id, g.players_needed - 1);
            }
            write(out, reply, strlen(reply));
        }
        else if (strcmp(cmd, "JOIN") == 0) {
            int found = 0;
            for (int i = 0; i < game_count; i++) {
                if (!games[i].finished &&
                    strcmp(games[i].name, data) == 0 &&
                    games[i].players_connected < games[i].players_needed) {
                    games[i].players_connected++;
                    sprintf(reply,
                            "Подключение к игре '%s'. ID: %d",
                            games[i].name, games[i].id);
                    found = 1;
                    break;
                }
            }
            if (!found)
                sprintf(reply, "Игра не найдена или заполнена");
            write(out, reply, strlen(reply));
        }
        else if (strcmp(cmd, "FIND") == 0) {
            int found = 0;
            for (int i = 0; i < game_count; i++) {
                if (!games[i].finished &&
                    games[i].players_connected < games[i].players_needed) {
                    games[i].players_connected++;
                    sprintf(reply,
                            "Вы подключены к игре '%s'. ID: %d",
                            games[i].name, games[i].id);
                    found = 1;
                    break;
                }
            }
            if (!found)
                sprintf(reply, "Свободных игр нет");
            write(out, reply, strlen(reply));
        }
        else if (strcmp(cmd, "GUESS") == 0) {
            char *sep = strchr(data, '|');
            if (!sep) {
                sprintf(reply, "Ошибка формата GUESS");
                write(out, reply, strlen(reply));
                close(out);
                continue;
            }
            char id_str[16];
            strncpy(id_str, data, sep - data);
            id_str[sep - data] = '\0';
            int game_id = atoi(id_str);
            char *guess = sep + 1;
            game_t *g = NULL;
            for (int i = 0; i < game_count; i++) {
                if (games[i].id == game_id) {
                    g = &games[i];
                    break;
                }
            }
            if (!g || g->finished) {
                sprintf(reply, "Игра не найдена или завершена");
            } else if (g->players_connected < g->players_needed) {
                sprintf(reply, "Ожидание игроков");
            } else {
                int bulls, cows;
                evaluate_guess(g->secret, guess, &bulls, &cows);
                if (bulls == 4) {
                    g->finished = 1;
                    sprintf(reply, "Победа! Число угадано");
                } else {
                    sprintf(reply, "%d быков, %d коров", bulls, cows);
                }
            }
            write(out, reply, strlen(reply));
        }
        else if (strcmp(cmd, "EXIT") == 0) {
            sprintf(reply, "Выход");
            write(out, reply, strlen(reply));
            unlink(client_fifo);
        }
        close(out);
    }
}