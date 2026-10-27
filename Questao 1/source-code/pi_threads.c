/*
 * TP1 - Questao 1: calculo de pi com N threads POSIX (serie de Leibniz-Gregory)
 *
 * pi/4 = sum_{n=0}^{inf} (-1)^n / (2n+1)
 *
 * Compilar: gcc -O2 -o pi_threads pi_threads.c -lm -lpthread
 * Executar: ./pi_threads <num_threads> [num_termos]
 *
 * Decisoes de projeto:
 *  - O espaco de calculo [0, TOTAL) e dividido uniformemente em N faixas
 *    contiguas; a thread i calcula os termos [i*chunk, (i+1)*chunk).
 *    O resto da divisao (se houver) e distribuido entre as primeiras threads.
 *  - Sem sincronizacao: cada thread escreve apenas em result[i].
 *  - Dentro do laco, so se usam variaveis locais (pilha da thread); o
 *    resultado e copiado para result[i] uma unica vez, no final.
 *  - A thread principal faz pthread_join em todas e soma result[].
 *  - result[] usa padding (uma linha de cache por posicao) para evitar
 *    false sharing na escrita final.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define DEFAULT_TERMS 1000000000UL   /* 10^9 termos */
#define MAX_THREADS   256
#define CACHE_LINE    64

typedef struct {
    double value;
    char   pad[CACHE_LINE - sizeof(double)];
} slot_t;

typedef struct {
    int           id;
    unsigned long start;   /* primeiro termo (inclusive) */
    unsigned long end;     /* ultimo termo (exclusive)   */
} task_t;

static slot_t result[MAX_THREADS];

static int parse_positive_int(const char *text, int *value)
{
    char *end;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        parsed < 1 || parsed > MAX_THREADS)
        return 0;

    *value = (int)parsed;
    return 1;
}

static int parse_terms(const char *text, unsigned long *value)
{
    char *end;
    unsigned long parsed;

    if (text[0] == '-')
        return 0;

    errno = 0;
    parsed = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed == 0)
        return 0;

    *value = parsed;
    return 1;
}

static void *worker(void *arg)
{
    task_t *t = (task_t *)arg;

    /* copias locais (pilha da thread) */
    unsigned long n     = t->start;
    unsigned long end   = t->end;
    int           id    = t->id;
    double        sum   = 0.0;
    double        sign  = (n % 2 == 0) ? 1.0 : -1.0;

    for (; n < end; n++) {
        sum  += sign / (2.0 * (double)n + 1.0);
        sign  = -sign;
    }

    result[id].value = sum;   /* unica escrita em memoria compartilhada */
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <num_threads> [num_termos]\n", argv[0]);
        return 1;
    }

    int nthreads;
    unsigned long total = DEFAULT_TERMS;

    if (!parse_positive_int(argv[1], &nthreads)) {
        fprintf(stderr, "num_threads deve estar entre 1 e %d\n", MAX_THREADS);
        return 1;
    }
    if (argc >= 3 && !parse_terms(argv[2], &total)) {
        fprintf(stderr, "num_termos deve ser um inteiro positivo\n");
        return 1;
    }
    if (argc > 3) {
        fprintf(stderr, "Uso: %s <num_threads> [num_termos]\n", argv[0]);
        return 1;
    }

    pthread_t tid[MAX_THREADS];
    task_t    task[MAX_THREADS];

    unsigned long chunk = total / nthreads;
    unsigned long rest  = total % nthreads;
    unsigned long pos   = 0;

    int created = 0;
    for (int i = 0; i < nthreads; i++) {
        unsigned long len = chunk + ((unsigned long)i < rest ? 1 : 0);
        task[i].id    = i;
        task[i].start = pos;
        task[i].end   = pos + len;
        pos += len;

        int error = pthread_create(&tid[i], NULL, worker, &task[i]);
        if (error != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            for (int j = 0; j < created; j++)
                pthread_join(tid[j], NULL);
            return 1;
        }
        created++;
    }

    for (int i = 0; i < nthreads; i++)
        pthread_join(tid[i], NULL);

    double pi = 0.0;
    for (int i = 0; i < nthreads; i++)
        pi += result[i].value;
    pi *= 4.0;

    printf("threads=%d termos=%lu pi=%.12f\n", nthreads, total, pi);
    return 0;
}
