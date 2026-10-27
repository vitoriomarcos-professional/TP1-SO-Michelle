/*
 * B.c - Processo B: recebe n1 e n2 de A, espera 1 s e devolve a soma.
 *
 * Compilar: gcc -Wall -Wextra B.c -o B -lrt
 * Uso:      ./B            (executa ate Ctrl+C; pode iniciar antes ou depois de A)
 *
 * B responde na fila indicada no proprio pedido, entao varios B e varios A
 * podem coexistir sem respostas trocadas.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>

#define FILA "/tp1_so_pedidos_caio"   /* mesmo nome usado em A.c */

typedef struct {
    int n1, n2;
    char resposta[64];
} Pedido;

typedef struct {
    int soma;
} Resposta;

volatile sig_atomic_t parar = 0;

void sinal_handler(int s) {
    (void)s;
    parar = 1;
}

int main(void) {
    struct sigaction sa = {0};
    sa.sa_handler = sinal_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* B tambem cria a fila (mesmos atributos de A), entao a ordem de inicio nao importa */
    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Pedido);

    mqd_t pedidos = mq_open(FILA, O_CREAT | O_RDONLY, 0600, &attr);
    if (pedidos == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    while (!parar) {
        Pedido p;
        if (mq_receive(pedidos, (char *)&p, sizeof(p), NULL) == -1) {
            if (errno == EINTR && parar) break;
            if (errno == EINTR) continue;
            perror("mq_receive");
            break;
        }

        p.resposta[sizeof(p.resposta) - 1] = '\0';   /* garante string valida */

        Resposta r;
        r.soma = p.n1 + p.n2;

        sleep(1);   /* "aguarda 1 segundo" */

        mqd_t resposta = mq_open(p.resposta, O_WRONLY);
        if (resposta == (mqd_t)-1) {
            perror("mq_open resposta");   /* A ja terminou */
            continue;
        }
        if (mq_send(resposta, (char *)&r, sizeof(r), 0) == -1)
            perror("mq_send resposta");
        mq_close(resposta);

        printf("B[%d]: %s -> %d + %d = %d\n", (int)getpid(), p.resposta, p.n1, p.n2, r.soma);
        fflush(stdout);
    }

    mq_close(pedidos);
    return 0;
}
