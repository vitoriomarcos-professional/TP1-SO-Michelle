/*
 * A.c - Processo A: sorteia n1 e n2, envia a B e imprime a soma recebida.
 *
 * Compilar: gcc -Wall -Wextra A.c -o A -lrt
 * Uso:      ./A            (executa até Ctrl+C)
 *           ./A limpar     (remove a fila de pedidos do sistema)
 *
 * Cada A cria a sua propria fila de resposta (nome com o pid) e informa esse
 * nome no pedido; assim B responde sempre ao A que fez o pedido.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>

#define FILA "/tp1_so_pedidos_caio"   /* nome exclusivo do grupo */

typedef struct {
    int n1, n2;
    char resposta[64];   /* nome da fila onde B deve responder */
} Pedido;

typedef struct {
    int soma;
} Resposta;

volatile sig_atomic_t parar = 0;

void sinal_handler(int s) {
    (void)s;
    parar = 1;
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "limpar") == 0) {
        if (mq_unlink(FILA) == -1 && errno != ENOENT)
            perror("mq_unlink");
        return 0;
    }

    /* sem SA_RESTART: mq_send/mq_receive retornam EINTR e o laco termina */
    struct sigaction sa = {0};
    sa.sa_handler = sinal_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    char nome[64];
    snprintf(nome, sizeof(nome), "/tp1_so_resp_%ld", (long)getpid());

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Pedido);

    mqd_t pedidos = mq_open(FILA, O_CREAT | O_WRONLY, 0600, &attr);
    if (pedidos == (mqd_t)-1) {
        perror("mq_open pedidos");
        return 1;
    }

    struct mq_attr attr_resp = {0};
    attr_resp.mq_maxmsg = 10;
    attr_resp.mq_msgsize = sizeof(Resposta);

    mqd_t respostas = mq_open(nome, O_CREAT | O_EXCL | O_RDONLY, 0600, &attr_resp);
    if (respostas == (mqd_t)-1) {
        perror("mq_open respostas");
        mq_close(pedidos);
        return 1;
    }

    srand((unsigned int)getpid());

    while (!parar) {
        Pedido p = {0};
        Resposta r;

        p.n1 = rand() % 100;
        p.n2 = rand() % 100;
        snprintf(p.resposta, sizeof(p.resposta), "%s", nome);

        if (mq_send(pedidos, (char *)&p, sizeof(p), 0) == -1) {
            if (errno == EINTR && parar) break;
            perror("mq_send");
            break;
        }
        printf("A[%d]: %d + %d = ...\n", (int)getpid(), p.n1, p.n2);
        fflush(stdout);

        if (mq_receive(respostas, (char *)&r, sizeof(r), NULL) == -1) {
            if (errno == EINTR && parar) break;
            perror("mq_receive");
            break;
        }
        printf("A[%d]: %d + %d = %d\n", (int)getpid(), p.n1, p.n2, r.soma);
        fflush(stdout);
    }

    mq_close(pedidos);
    mq_close(respostas);
    mq_unlink(nome);          /* remove a fila de resposta deste A */
    return 0;
}
