# TP1 de Sistemas Operacionais: cálculo de π com threads

Implementação em C da série de Leibniz–Gregory, com divisão do trabalho entre threads POSIX e coleta de tempos de execução.

## Arquivos

- `pi_threads.c`: implementação do cálculo paralelo.
- `bench.ps1`: benchmark para Windows com MSYS2 UCRT64; grava os tempos em CSV.
- `bench.sh`: benchmark para Linux ou ambiente Bash com GCC e pthreads.
- `analise.py`: calcula estatísticas e gera tabelas e gráficos a partir de CSVs.
- `relatorio_questao1.md`: relatório dos experimentos disponíveis.
- `tempos_*.csv`, `grafico_*.png`, `tabelas.md`: dados e saídas experimentais referenciados no relatório.

## Windows com MSYS2

No terminal **MSYS2 UCRT64**, instale o compilador e a biblioteca pthreads:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

Adicione `C:\msys64\ucrt64\bin` ao `PATH` do Windows. No PowerShell, na pasta do projeto, compile e execute casos curtos:

```powershell
gcc -O2 -Wall -Wextra -o pi_threads.exe pi_threads.c -lm -lpthread
.\pi_threads.exe 1 1000000
.\pi_threads.exe 4 1000003
```

O executável deve imprimir uma aproximação de π. Para executar o benchmark completo, com um bilhão de termos e cinco repetições por configuração:

```powershell
.\bench.ps1 -Platform nome_da_maquina
```

O script testa 1, 2, 4, 8, 16 e 32 threads e cria `tempos_nome_da_maquina.csv` no diretório atual. Para testar apenas algumas quantidades:

```powershell
.\bench.ps1 -Platform nome_da_maquina -ThreadCounts 1,2,4
```

## Análise dos CSVs

Instale as dependências Python:

```powershell
py -m pip install pandas matplotlib tabulate
```

Analise um ou mais computadores, usando um CSV por plataforma:

```powershell
py analise.py tempos_windows_pc1.csv tempos_computador2.csv
```

A análise gera `tabelas.md`, gráficos de tempo médio por plataforma e o gráfico normalizado. CV acima de 5% é sinalizado; refaça essas medições antes de usá-las como resultados estatisticamente aceitáveis. Evite misturar tentativas de medição no mesmo CSV sem identificar claramente a coleta.

## Resultados atuais

Os dados versionados são de uma máquina Windows 11 com Intel Core i5-8250U (4 núcleos e 8 processadores lógicos). Nesta coleta, os grupos de 8, 16 e 32 threads ficaram abaixo de 5% de CV; 1, 2 e 4 threads não atingiram o limite, inclusive após repetição. O relatório identifica essas limitações. Ainda falta adicionar resultados de uma segunda máquina para completar a comparação solicitada no trabalho.

## Publicação no GitHub

Crie um repositório vazio no GitHub e, nesta pasta, configure o endereço remoto indicado pelo GitHub:

```powershell
git remote add origin https://github.com/SEU_USUARIO/NOME_DO_REPOSITORIO.git
git branch -M main
git push -u origin main
```

Antes do primeiro envio, revise o `git status` e substitua no relatório a indicação de URL pendente pelo endereço público real do repositório.