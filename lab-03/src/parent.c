#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

static const size_t SHARED_SIZE = 4096;

int main(void) {
    char outname[256];
    printf("Enter output filename: ");
    if (fgets(outname, sizeof(outname), stdin) == NULL) {
        fprintf(stderr, "Error reading filename\n");
        return 1;
    }
    
    size_t len = strlen(outname);
    if (len > 0 && outname[len - 1] == '\n') outname[len - 1] = '\0';
    if (strlen(outname) == 0) {
        fprintf(stderr, "Filename cannot be empty\n");
        return 1;
    }

    const char* shared_file = "shared.dat";

    int fd = open(shared_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) { perror("open"); return 1; }

    if (ftruncate(fd, SHARED_SIZE) < 0) { perror("ftruncate"); close(fd); return 1; }

    void* shared = mmap(NULL, SHARED_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) { perror("mmap"); close(fd); return 1; }
    close(fd);

    pid_t pid = fork();
    if (pid == -1) { perror("fork"); munmap(shared, SHARED_SIZE); return 1; }

    if (pid == 0) {
        execl("./child", "child", outname, shared_file, NULL);
        perror("exec"); munmap(shared, SHARED_SIZE); return 1;
    }

    usleep(100000);

    printf("Enter float numbers (empty line to exit):\n");

    char line[SHARED_SIZE];
    while (1) {
        if (!fgets(line, sizeof(line), stdin)) break;
        len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if (strlen(line) == 0) break;

        memset(shared, 0, SHARED_SIZE);
        memcpy(shared, line, strlen(line));

        if (kill(pid, SIGUSR1) < 0) perror("kill");
    }

    if (kill(pid, SIGTERM) < 0) perror("kill");
    int status;
    waitpid(pid, &status, 0);
    munmap(shared, SHARED_SIZE);

    return 0;
}