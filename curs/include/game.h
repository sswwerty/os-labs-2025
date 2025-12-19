#ifndef GAME_H
#define GAME_H
typedef struct {
    int id;
    char name[64];
    int players_needed;
    int players_connected;
    char secret[5];
    int finished;
} game_t;
void generate_secret(char *secret);
void evaluate_guess(const char *secret, const char *guess, int *bulls, int *cows);
#endif
