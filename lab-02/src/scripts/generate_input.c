// generate_input.c
// Usage: gcc -O2 -o generate_input generate_input.c
// ./generate_input N out.bin
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s N out.bin\n", argv[0]);
        return 1;
    }
    long N = atol(argv[1]);
    const char *out = argv[2];
    FILE *f = fopen(out, "wb");
    if (!f) { perror("fopen"); return 1; }
    srand((unsigned)time(NULL));
    for (long i = 0; i < N; ++i) {
        int x = rand();
        fwrite(&x, sizeof(int), 1, f);
    }
    fclose(f);
    return 0;
}