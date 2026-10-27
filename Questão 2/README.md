# questao2SO

Questão 2 do Trabalho Prático 1 de Sistemas Operacionais (CEFET-MG, Profa. Michelle Hanne): dois processos independentes, A e B, que se comunicam por filas de mensagens POSIX.

- `A.c`: sorteia n1 e n2 (entre 0 e 99), envia a B e imprime a soma recebida.
- `B.c`: recebe n1 e n2, espera 1 segundo e devolve a soma a A.

## Compilar

    gcc -Wall -Wextra A.c -o A -lrt
    gcc -Wall -Wextra B.c -o B -lrt

## Executar

Em terminais diferentes (pode haver mais de um A e mais de um B):

    ./B
    ./A

Para encerrar, Ctrl+C em cada terminal.

## Limpeza

A fila de pedidos (`/tp1_so_pedidos_caio`) continua existindo no sistema depois que os processos terminam. Para removê-la:

    ./A limpar

Requer Linux (as filas aparecem em `/dev/mqueue`).
EOF