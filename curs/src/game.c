#include "game.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
void generate_secret(char *secret) {
    srand(time(NULL));
    int used[10] = {0};
    for (int i = 0; i < 4; i++) {
        int digit;
        do {
            digit = rand() % 10;
        } while (used[digit]);
        used[digit] = 1;
        secret[i] = '0' + digit;
    }
    secret[4] = '\0';
}
void evaluate_guess(const char *secret, const char *guess, int *bulls, int *cows) {
    *bulls = 0;
    *cows = 0;
    int used_secret[4] = {0};
    int used_guess[4] = {0};
    // Подсчёт быков
    for (int i = 0; i < 4; i++) {
        if (secret[i] == guess[i]) {
            (*bulls)++;
            used_secret[i] = 1;
            used_guess[i] = 1;
        }
    }
    // Подсчёт коров
    for (int i = 0; i < 4; i++) {
        if (used_guess[i]) continue;
        for (int j = 0; j < 4; j++) {
            if (used_secret[j]) continue;
            if (guess[i] == secret[j]) {
                (*cows)++;
                used_secret[j] = 1;
                break;
            }
        }
    }
}