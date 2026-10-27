# Trabalho Prático 1 – Sistemas Operacionais

Centro Federal de Educação Tecnológica de Minas Gerais (CEFET-MG) – Profa. Michelle Hanne

## Questão 2: Filas de mensagens POSIX

### Decisões de projeto e implementação

O sistema é formado por dois programas independentes em C, A (`A.c`) e B (`B.c`), que se comunicam por filas de mensagens POSIX (`mqueue.h`) por meio de `mq_open`, `mq_send`, `mq_receive`, `mq_close` e `mq_unlink`. Os programas são compilados com `gcc ... -lrt`. O código-fonte completo está disponível em [https://github.com/vitoriomarcos-professional/TP1-SO-Michelle](https://github.com/vitoriomarcos-professional/TP1-SO-Michelle).

- **Número de filas:** uma fila de pedidos compartilhada (`/tp1_so_pedidos_caio`), usada por todos os processos A para enviar pedidos e por todos os B para recebê-los, e uma fila de resposta para cada processo A (`/tp1_so_resp_<pid>`). Com N processos A há, portanto, N + 1 filas.
- **Formato das mensagens:** o pedido (`Pedido`, 72 bytes) contém `n1`, `n2` e o nome da fila onde a resposta deve ser entregue; a resposta (`Resposta`, 4 bytes) contém apenas a soma. Enviar `n1` e `n2` na mesma mensagem garante que dois processos B nunca dividam entre si os números de um mesmo pedido.
- **Processo A:** cria a fila de pedidos (se ainda não existir) e a sua própria fila de resposta, sorteia `n1` e `n2` entre 0 e 99, envia o pedido com o nome da sua fila de resposta, bloqueia esperando a soma e a imprime. Cada linha de saída traz o PID do processo.
- **Processo B:** abre a fila de pedidos (criando-a, com os mesmos atributos de A, se ainda não existir, o que torna irrelevante a ordem de início dos processos), recebe um pedido, espera 1 segundo (`sleep(1)`, como no enunciado), abre a fila de resposta indicada no pedido, envia a soma e a fecha. Se o A de origem já tiver terminado, B registra o erro e segue para o próximo pedido.
- **Encerramento:** `SIGINT` e `SIGTERM` são tratados sem `SA_RESTART`, de modo que `mq_send` e `mq_receive` retornam `EINTR` e o laço termina. Cada A remove a própria fila de resposta com `mq_unlink`. O comando `./A limpar` remove a fila de pedidos do sistema.
- **Nomes das filas:** exclusivos do grupo, para evitar conflito com as filas de outros alunos em `/dev/mqueue`.

### Metodologia e resultados

Foram executados os quatro cenários do roteiro (1 A + 1 B, 2 A + 1 B, 1 A + 2 B e 2 A + 2 B), cada um por 15 segundos, em um notebook com Ubuntu 22.04 (kernel Linux 6.8.0-138), `gcc` 12.3.0 e 8 processadores lógicos. A saída de cada processo foi redirecionada para um arquivo de log, e as respostas completas (linhas `A[pid]: n1 + n2 = soma`) e os pedidos atendidos (linhas `B[pid]: ...`) foram contados com `grep`. A correção da soma impressa em cada resposta de A foi conferida linha a linha com `awk`. Os resultados estão na Tabela 1.

**Tabela 1 – Resultados dos quatro cenários (15 s por cenário)**

| Cenário | Respostas recebidas (A) | Pedidos atendidos (B) | Respostas incorretas |
|---|---:|---:|---:|
| 1 A + 1 B | 14 | 14 | 0 |
| 2 A + 1 B | 8 + 7 | 16 | 0 |
| 1 A + 2 B | 14 | 8 + 7 | 0 |
| 2 A + 2 B | 14 + 14 | 14 + 14 | 0 |

Nos cenários 2 A + 1 B e 1 A + 2 B, B atendeu um pedido a mais do que as respostas impressas pelos A (16 contra 15 e 15 contra 14). A diferença vem do corte da execução aos 15 s: o último pedido de um dos A ainda estava em andamento, isto é, B já havia enviado a soma, mas A foi encerrado antes de imprimi-la (o log de A termina em uma linha `A[pid]: n1 + n2 = ...`, sem a soma). Não se trata de falha de comunicação.

### Análise

- **1 A + 1 B:** a vazão é de cerca de 1 soma por segundo, limitada pelo `sleep(1)` de B (14 somas em 15 s). Cada soma impressa por A corresponde aos seus próprios números.
- **2 A + 1 B:** B atende um pedido por segundo no total, então a vazão do sistema não aumenta e cada A recebe cerca de metade das respostas do cenário anterior (8 e 7, contra 14). B é o gargalo, e os pedidos aguardam na fila de pedidos; B atendeu 8 pedidos de cada A.
- **1 A + 2 B:** não há ganho de desempenho: A recebeu 14 respostas, o mesmo número do cenário com um único B. A só envia o pedido seguinte depois de receber a soma, então há no máximo um pedido em atendimento. Os pedidos foram divididos de forma quase igual entre os dois B (8 e 7), mas a cada instante apenas um deles atende enquanto o outro espera em `mq_receive`, de modo que cada B fica ocioso cerca de metade do tempo e a vazão total não muda.
- **2 A + 2 B:** a vazão total dobra (cerca de 2 somas por segundo: 28 somas em 15 s, 14 para cada A e 14 atendidas por cada B), pois os dois B atendem pedidos em paralelo e os dois A mantêm um pedido cada em andamento. Em todos os cenários, cada A recebeu apenas as somas dos seus próprios pedidos.

### Problemas de concorrência e solução adotada

Com mais de um processo A, o principal risco de concorrência está na fila de respostas. Se todos os A esperassem em uma única fila de respostas compartilhada, uma soma enviada por B poderia ser lida por qualquer A que estivesse bloqueado em `mq_receive`, e não necessariamente por aquele que fez o pedido, o que faria um A imprimir uma soma que não corresponde aos seus números. Com mais de um B, haveria ainda o risco de dois B dividirem os dois números de um mesmo pedido, se `n1` e `n2` fossem enviados em mensagens separadas.

O projeto evita esses problemas desde o início: cada A cria uma fila de resposta exclusiva, cujo nome contém o seu PID, e informa esse nome no pedido; B responde diretamente nessa fila, de modo que a soma só pode ser lida pelo A correto. Os dois números seguem juntos em uma única mensagem. Como resultado, nenhum dos quatro cenários apresentou resposta incorreta (Tabela 1): em todas as respostas de A, a soma impressa era igual a `n1 + n2`. Essa análise do problema da fila compartilhada é de natureza teórica, pois a versão com fila de resposta única não foi implementada nem medida neste trabalho.

Outros cuidados adotados foram: a criação da fila de pedidos por A e por B, com os mesmos atributos, para que a ordem de início não importe; a remoção da fila de resposta por cada A ao terminar, evitando filas órfãs em `/dev/mqueue`; e o tratamento da ausência da fila de resposta em B, para o caso de o A de origem já ter encerrado.
