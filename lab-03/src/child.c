#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

static char* shared_data = NULL;
static const size_t SHARED_SIZE = 4096;
static int should_exit = 0;
static FILE* outfile = NULL;

void handle_signal(int sig) {
    if (sig == SIGTERM) { should_exit = 1; return; }
    if (!shared_data) return;

    char buffer[SHARED_SIZE];
    memcpy(buffer, shared_data, SHARED_SIZE);

    float num, sum = 0;
    char* token;
    char* rest = buffer;

    while ((token = strtok_r(rest, " \t\n", &rest)) != NULL) {
        if (sscanf(token, "%f", &num) == 1) sum += num;
    }

    fprintf(outfile, "Sum: %f\n", sum);
    fflush(outfile);
}

int main(int argc, char* argv[]) {
    if (argc != 3) { fprintf(stderr, "Usage: child <output_file> <shared_file>\n"); return 1; }

    const char* outname = argv[1];
    const char* shared_name = argv[2];

    outfile = fopen(outname, "w");
    if (!outfile) { perror("fopen"); return 1; }

    int fd = open(shared_name, O_RDONLY);
    if (fd < 0) { perror("open shared file"); fclose(outfile); return 1; }

    shared_data = mmap(NULL, SHARED_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (shared_data == MAP_FAILED) { perror("mmap"); close(fd); fclose(outfile); return 1; }

    close(fd);

    signal(SIGUSR1, handle_signal);
    signal(SIGTERM, handle_signal);

    while (!should_exit) pause();

    if (shared_data != MAP_FAILED) munmap(shared_data, SHARED_SIZE);
    if (outfile) fclose(outfile);

    return 0;
}