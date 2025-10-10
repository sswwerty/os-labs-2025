#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int value; 
} sem_compat_t;

int sem_compat_init(sem_compat_t *s, unsigned int value) {
    if (!s) return -1;
    if (pthread_mutex_init(&s->mutex, NULL) != 0) return -1;
    if (pthread_cond_init(&s->cond, NULL) != 0) {
        pthread_mutex_destroy(&s->mutex);
        return -1;
    }
    s->value = (int)value;
    return 0;
}

int sem_compat_trywait(sem_compat_t *s) {
    if (!s) return -1;
    if (pthread_mutex_lock(&s->mutex) != 0) return -1;
    if (s->value > 0) {
        s->value--;
        pthread_mutex_unlock(&s->mutex);
        return 0;
    } else {
        pthread_mutex_unlock(&s->mutex);
        return -1;
    }
}

int sem_compat_post(sem_compat_t *s) {
    if (!s) return -1;
    if (pthread_mutex_lock(&s->mutex) != 0) return -1;
    s->value++;
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->mutex);
    return 0;
}

int sem_compat_destroy(sem_compat_t *s) {
    if (!s) return -1;
    pthread_mutex_destroy(&s->mutex);
    pthread_cond_destroy(&s->cond);
    s->value = 0;
    return 0;
}

/* ------------------------------------------------------ */

typedef struct {
    int *arr;
    long left;
    long right;
} qargs_t;

sem_compat_t thread_sem;
atomic_int active_threads = 0;

static inline int cmp_int(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

static long partition(int *arr, long l, long r) {
    int pivot = arr[r];
    long i = l - 1;
    for (long j = l; j < r; ++j) {
        if (arr[j] <= pivot) {
            ++i;
            int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
        }
    }
    int tmp = arr[i+1]; arr[i+1] = arr[r]; arr[r] = tmp;
    return i + 1;
}

void *qsort_thread_func(void *arg);

void qsort_mt(int *arr, long l, long r) {
    if (l >= r) return;
    const long THRESH = 1000; // if subarray small -> single-threaded
    if (r - l + 1 <= THRESH) {
        qsort(arr + l, (size_t)(r - l + 1), sizeof(int), cmp_int);
        return;
    }

    long pi = partition(arr, l, r);

    pthread_t tid;
    qargs_t *args = NULL;
    int created = 0;

    if (sem_compat_trywait(&thread_sem) == 0) {
        args = malloc(sizeof(qargs_t));
        if (!args) {
            sem_compat_post(&thread_sem);
            args = NULL;
            created = 0;
        } else {
            args->arr = arr;
            args->left = l;
            args->right = pi - 1;
            atomic_fetch_add(&active_threads, 1);
            if (pthread_create(&tid, NULL, qsort_thread_func, args) == 0) {
                created = 1;
            } else {
                atomic_fetch_sub(&active_threads, 1);
                sem_compat_post(&thread_sem);
                free(args);
                created = 0;
            }
        }
    }

    qsort_mt(arr, pi + 1, r);

    if (created) {
        pthread_join(tid, NULL);
    } else {
        qsort_mt(arr, l, pi - 1);
    }
}

void *qsort_thread_func(void *arg) {
    qargs_t *a = (qargs_t*)arg;
    if (a) {
        qsort_mt(a->arr, a->left, a->right);
        free(a);
    }
    atomic_fetch_sub(&active_threads, 1);
    sem_compat_post(&thread_sem);
    return NULL;
}

double timespec_diff_sec(const struct timespec *start, const struct timespec *end) {
    double s = (double)(end->tv_sec - start->tv_sec);
    double ns = (double)(end->tv_nsec - start->tv_nsec);
    return s + ns / 1e9;
}

void print_usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s [-n N] [-m MAX_THREADS] [-r SEED] [-o OUT_CSV] [-v]\n"
            "  -n N            number of elements (default 1000000)\n"
            "  -m MAX_THREADS  maximum concurrent threads (including main) (default 4)\n"
            "  -r SEED         random seed (default time)\n"
            "  -o OUT_CSV      append results to CSV\n"
            "  -v              verbose (print diagnostics)\n",
            prog);
}

int main(int argc, char **argv) {
    long N = 1000000;
    int max_threads = 4;
    unsigned int seed = (unsigned int)time(NULL);
    const char *out_csv = NULL;
    int verbose = 0;
    int opt;
    while ((opt = getopt(argc, argv, "n:m:r:o:vh")) != -1) {
        switch (opt) {
            case 'n': N = atol(optarg); break;
            case 'm': max_threads = atoi(optarg); break;
            case 'r': seed = (unsigned int)atoi(optarg); break;
            case 'o': out_csv = optarg; break;
            case 'v': verbose = 1; break;
            default: print_usage(argv[0]); return 1;
        }
    }

    if (N <= 0) { fprintf(stderr, "Invalid N\n"); return 1; }
    if (max_threads <= 0) max_threads = 1;

    int *arr = malloc(sizeof(int) * (size_t)N);
    if (!arr) { perror("malloc"); return 1; }

    srand(seed);
    for (long i = 0; i < N; ++i) arr[i] = rand();

    int sem_init_val = max_threads > 1 ? (max_threads - 1) : 0;
    if (sem_compat_init(&thread_sem, (unsigned)sem_init_val) != 0) {
        fprintf(stderr, "sem_compat_init failed\n");
        free(arr);
        return 1;
    }

    atomic_store(&active_threads, 1);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    qsort_mt(arr, 0, N - 1);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = timespec_diff_sec(&t0, &t1);

    // verify correctness
    int sorted = 1;
    for (long i = 1; i < N; ++i) {
        if (arr[i-1] > arr[i]) { sorted = 0; break; }
    }

    printf("N=%ld max_threads=%d seed=%u elapsed=%.6f sorted=%d active_threads=%d\n",
           N, max_threads, seed, elapsed, sorted, atomic_load(&active_threads));

    if (out_csv) {
        FILE *f = fopen(out_csv, "a");
        if (f) {
            fprintf(f, "%ld,%d,%u,%.9f,%d\n", N, max_threads, seed, elapsed, sorted);
            fclose(f);
        } else {
            fprintf(stderr, "Can't open %s: %s\n", out_csv, strerror(errno));
        }
    }

    sem_compat_destroy(&thread_sem);
    free(arr);
    if (verbose) {
        fprintf(stderr, "Finished. active_threads=%d\n", atomic_load(&active_threads));
    }
    return 0;
}