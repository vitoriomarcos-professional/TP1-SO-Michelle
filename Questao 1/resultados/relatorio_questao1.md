# Cálculo de π com threads POSIX

**Relatório da Questão 1 — TP1 de Sistemas Operacionais**  
**Autores:** preencher nomes do grupo  
**Data:** 26/10/2026

> **Situação dos resultados:** este relatório registra os experimentos executados em uma máquina. A comparação entre duas plataformas e o link público do código ainda dependem de ações externas; portanto, o documento não deve ser entregue como comparação final antes de completar esses itens.

## 1. Objetivo

Implementar em C uma aproximação de π pela série de Leibniz–Gregory, dividindo pelo menos um bilhão de termos entre threads POSIX. O estudo mede o tempo para 1, 2, 4, 8, 16 e 32 threads e avalia o efeito do paralelismo.

## 2. Projeto e implementação

Para cada termo, o programa calcula $(-1)^n/(2n+1)$ em precisão `double`. O intervalo total `[0, total)` é repartido em faixas contíguas: cada thread recebe o quociente da divisão, e o resto é distribuído uma unidade por vez às primeiras threads. A thread inicia o sinal de acordo com a paridade do seu primeiro índice, inclusive quando a faixa começa em índice ímpar.

Cada worker copia seus limites para variáveis locais, acumula uma soma privada e grava o resultado uma única vez em `result[id]`. Não há atualização compartilhada dentro do laço. Após `pthread_join` de todas as threads, a thread principal soma os resultados e multiplica por quatro. O vetor usa padding de 64 bytes por posição para reduzir false sharing. Os argumentos são validados e o número de threads permitido é de 1 a 256.

O código está em [pi_threads.c](pi_threads.c). A compilação usada foi `gcc -O2 -Wall -Wextra -o pi_threads.exe pi_threads.c -lm -lpthread`, com GCC 16.2.0 do MSYS2 UCRT64 e winpthreads. O executável foi testado no Windows; os testes curtos retornaram valores próximos de π, e entradas inválidas foram rejeitadas.

## 3. Ambiente e método

| Item | Máquina 1 |
|---|---|
| Sistema operacional | Windows 11 Home, 64 bits, versão 10.0.22631 |
| Processador | Intel Core i5-8250U, 1,60 GHz |
| Núcleos / processadores lógicos | 4 / 8 |
| Compilador | GCC 16.2.0, MSYS2 UCRT64 |
| Termos por execução | 1.000.000.000 |
| Repetições | 5 por configuração |
| Cronometragem | `Stopwatch` do PowerShell, tempo de parede do processo |

O benchmark está em [bench.ps1](bench.ps1). Ele executa cada configuração em um processo separado e grava os tempos em CSV. O uso de `Stopwatch` é a alternativa Windows à medição com `time` do Unix solicitada no enunciado.

## 4. Resultados da máquina 1

A tabela usa a coleta principal [tempos_windows_pc1.csv](tempos_windows_pc1.csv). O desvio-padrão é amostral; $CV=100\,s/\bar{t}$. O limite definido no enunciado é 5%.

| Threads | Média (s) | Desvio (s) | CV (%) | Avaliação |
|---:|---:|---:|---:|---|
| 1 | 3,396 | 0,617 | 18,18 | Refazer |
| 2 | 1,454 | 0,170 | 11,67 | Refazer |
| 4 | 0,696 | 0,038 | 5,50 | Refazer |
| 8 | 0,554 | 0,023 | 4,13 | Aceitável |
| 16 | 0,561 | 0,015 | 2,67 | Aceitável |
| 32 | 0,548 | 0,012 | 2,15 | Aceitável |

Os grupos de 1, 2 e 4 threads foram medidos novamente, sem combinar tentativas. A coleta seletiva está em [tempos_windows_pc1_retry.csv](tempos_windows_pc1_retry.csv): os CVs foram, respectivamente, 25,18%, 9,24% e 6,64%. Portanto, nenhuma das repetições tornou esses grupos aceitáveis; não se excluíram valores atípicos.

![Tempo médio por número de threads, coleta principal](grafico_windows_pc1.png)

![Tempo normalizado pela execução com uma thread](grafico_normalizado.png)

Os gráficos são exploratórios para os grupos com CV acima de 5%. A coleta principal sugere redução do tempo até 8 threads e estabilização posterior. A média passa de 0,554 s em 8 threads para 0,561 s em 16 e 0,548 s em 32; essa diferença pequena não demonstra ganho adicional consistente. A curva normalizada usa a média de 1 thread como referência, mas esse grupo tem CV de 18,18%; assim, seus quocientes devem ser tratados como indicativos, não como conclusão estatística.

O resultado numérico foi consistente entre as configurações: aproximadamente `3.141592652588` a `3.141592652590` para um bilhão de termos. A série alternada converge lentamente; o valor deve ser entendido como aproximação, não como cálculo exato de π.

## 5. Escalonamento e modelo de threads

**Preemptivas ou cooperativas?** No Windows, as threads nativas são escalonadas pelo kernel e podem ser interrompidas sem chamar voluntariamente `yield`. Portanto, para esta implementação com winpthreads sobre threads do Windows, são preemptivas. O worker mantém um laço de cálculo e não cede explicitamente a CPU.

**Modelo de threads.** O modelo esperado para winpthreads no Windows é 1:1: cada thread POSIX criada é representada por uma thread nativa do Windows. Os tempos mostram paralelismo efetivo nesta máquina, mas não provam sozinhos o mapeamento. Para a entrega final, confirme a implementação e registre a observação das threads do processo no Gerenciador de Tarefas ou em uma ferramenta de inspeção de threads. A máquina tem quatro núcleos e oito processadores lógicos, o que também ajuda a contextualizar o platô observado a partir de oito threads.

Referências técnicas: [Threads and Processes (Microsoft Learn)](https://learn.microsoft.com/en-us/windows/win32/procthread/threads-and-processes), [Scheduling Priorities (Microsoft Learn)](https://learn.microsoft.com/en-us/windows/win32/procthread/scheduling-priorities) e [winpthreads (mingw-w64)](https://github.com/mingw-w64/mingw-w64/tree/master/mingw-w64-libraries/winpthreads).

## 6. Conclusão e itens pendentes

Nesta máquina, o programa executou corretamente e houve redução expressiva de tempo em relação à execução sequencial nas configurações com mais threads. Porém, apenas 8, 16 e 32 threads passaram no CV máximo de 5% na coleta principal. A alta variação em 1, 2 e 4 threads impede uma comparação estatística final completa nesta plataforma.

Para concluir a entrega do enunciado, ainda é necessário: (1) obter CV de até 5% para 1, 2 e 4 threads, repetindo medições em condições de carga controlada; (2) executar os seis casos em um segundo computador de configuração distinta e registrar suas especificações; (3) gerar novamente os gráficos e a análise normalizada com as duas plataformas e dados aceitos; (4) publicar o código e substituir o campo abaixo pela URL real.

**URL pública do código-fonte:** pendente de publicação do repositório.  
**Plataforma 2:** medições e especificações pendentes.