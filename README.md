# Mini-SO — Simulador de Gerência de Processos e Memória

> Projeto da disciplina **Infraestrutura de Software** — 2° Grau de Qualificação  
> Profa. Monique Soares

---

## O que é este projeto?

Este programa simula o comportamento básico de um **Sistema Operacional (SO)** — o software que gerencia tudo no seu computador (Windows, Linux, macOS).

Quando você abre vários programas ao mesmo tempo, o SO precisa decidir qual roda na CPU agora, qual espera, e onde cada um fica na memória RAM. É exatamente isso que este simulador faz, só que de forma simplificada e visível no terminal.

---

## Como rodar

### Pré-requisitos
- GCC instalado (via MSYS2 no Windows, ou nativo no Linux/Mac)

### Compilar
```bash
gcc -Wall -std=c99 -o mini_so main.c pcb.c memoria.c escalonador.c log.c
```

### Executar
```bash
./mini_so        # Linux/Mac
.\mini_so.exe    # Windows
```

O programa pergunta se quer ler de arquivo ou teclado.  
Para usar o arquivo de exemplo: digite `A` → `processos.txt`

---

## Estrutura de arquivos

```
mini-so/
├── main.c          → Ponto de entrada. Controla o loop principal da simulação
├── pcb.h           → Define a estrutura do processo (PCB)
├── pcb.c           → Implementa funções auxiliares do PCB
├── memoria.h       → Interface do módulo de memória RAM
├── memoria.c       → Implementa a RAM simulada e o algoritmo First-Fit
├── escalonador.h   → Interface do escalonador Round-Robin
├── escalonador.c   → Implementa as filas e o algoritmo de escalonamento
├── log.h           → Interface do módulo de log
├── log.c           → Grava eventos e estado final em arquivo .txt
├── Makefile        → Atalho para compilar tudo com um só comando
└── processos.txt   → Arquivo de entrada com os processos a simular
```

---

## Glossário — cada nome importante explicado

### Conceitos de Sistema Operacional

| Termo | O que significa |
|-------|----------------|
| **SO (Sistema Operacional)** | Software que gerencia hardware e programas. Ex: Windows, Linux |
| **Processo** | Um programa em execução. Cada aba do Chrome é um processo |
| **CPU (Processador)** | O "cérebro" do computador. Só executa um processo por vez (simplificado) |
| **RAM** | Memória rápida e temporária. Todo processo precisa de espaço nela para rodar |
| **Escalonamento** | A arte de decidir qual processo usa a CPU agora e qual espera |
| **Troca de Contexto** | Quando a CPU para um processo e começa outro. O SO salva o "estado" de cada um |
| **Deadlock** | Situação onde dois processos ficam esperando um pelo outro para sempre, travando tudo |

---

### A estrutura PCB — Process Control Block

O PCB é como uma **ficha cadastral** de cada processo. O SO guarda uma dessas para cada programa rodando.

```c
typedef struct {
    int   pid;            // Número de identificação único do processo
    int   burst_total;    // Tempo total que ele precisa de CPU (em segundos)
    int   burst_restante; // Quanto tempo ainda falta para terminar
    int   tamanho_ram;    // Quantos bytes ele ocupa na RAM
    int   prioridade;     // Quão urgente ele é (número menor = mais urgente)
    int   base_ram;       // Em qual posição da RAM ele começa (-1 = fora da RAM)
    Estado estado;        // Em que situação ele está agora
} PCB;
```

| Campo | Exemplo | O que significa na prática |
|-------|---------|---------------------------|
| `pid` | 1, 2, 3 | Como o RG de um cidadão — identifica o processo |
| `burst_total` | 10 | O processo precisa de 10 segundos de CPU no total |
| `burst_restante` | 6 | Já rodou 4s, ainda faltam 6s |
| `tamanho_ram` | 200 | Precisa de 200 bytes de RAM para funcionar |
| `prioridade` | 2 | Nível de urgência (usado para ordenar a fila) |
| `base_ram` | 0 | Começa na posição 0 da RAM. -1 significa que ainda não entrou |
| `estado` | PRONTO | Situação atual — veja abaixo |

---

### Os estados de um processo

Um processo passa por vários estados durante sua vida, como um pedido de delivery:

```
NOVO → PRONTO → EXECUTANDO → ENCERRADO
                    ↕
                ESPERANDO (sem RAM disponível)
```

| Estado | Analogia | O que está acontecendo |
|--------|----------|----------------------|
| `NOVO` | Pedido feito | Processo foi lido do arquivo, ainda não entrou no sistema |
| `PRONTO` | Pedido na fila | Está na RAM, esperando sua vez na CPU |
| `EXECUTANDO` | Pedido sendo preparado | Está usando a CPU agora |
| `ESPERANDO` | Pedido em espera de ingrediente | Quer entrar mas não tem RAM disponível |
| `ENCERRADO` | Pedido entregue | Terminou de executar, saiu da RAM |

---

### Módulo 1 — Escalonamento Round-Robin

**O que é:** O algoritmo que decide a ordem de execução dos processos na CPU.

**Como funciona o Round-Robin:**  
Imagina uma fila de pessoas esperando para usar um micro-ondas. Cada pessoa tem direito a usar por **2 minutos** (o Quantum). Se não terminou de esquentar, vai para o fim da fila e espera de novo. Assim todo mundo tem vez, ninguém monopoliza.

| Termo | Valor | Significado |
|-------|-------|-------------|
| `QUANTUM` | 2 segundos | Tempo máximo que cada processo usa a CPU por rodada |
| `fila_prontos` | — | Fila circular de processos esperando a CPU |
| `fila_espera` | — | Processos que não cabem na RAM ainda |
| `MAX_PROCESSOS` | 64 | Máximo de processos que o sistema suporta |

**Fila circular:** Quando o último lugar é ocupado, o próximo vai para a posição 0 de novo. Evita desperdício de memória.

---

### Módulo 2 — Gerência de Memória (First-Fit)

**O que é:** Simula 1 KB (1024 bytes) de RAM usando um array de inteiros.

```c
int ram[1024];  // 0 = livre, N = ocupado pelo processo de PID N
```

**Visualização:**
```
Posição:  [0][1][2]...[199][200]...[299][300]...[811][812]...[939][940]...[1023]
Conteúdo:  [1] [1] [1]...[1]  [2] ...[2]  [3] ...[3]  [4] ...[4]  [0] ...[0]
            ←— PID 1: 200B —→  ←— PID 2: 100B —→  ←— PID 3 —→  ←— PID 4 —→  Livre
```

**Algoritmo First-Fit (Primeiro Encaixe):**  
Procura da posição 0 até o final e aloca no **primeiro buraco** que couber o processo. Simples e rápido.

```
RAM: [P1: 200B][LIVRE: 100B][P3: 512B][LIVRE: 212B]
                    ↑
      First-Fit coloca o novo processo aqui se couber
```

Se não couber em lugar nenhum → processo vai para a `fila_espera`.  
Quando algum processo termina e libera RAM → sistema tenta admitir os que estavam esperando.

---

### Módulo 3 — Log em Arquivo

Ao final da simulação, o programa grava o arquivo `simulacao_log.txt` com:
- Cada evento com seu timestamp (`[t=006s] PID 4 ENCERRADO`)
- Mapa final da memória com endereços
- Tabela resumo de todos os processos

---

### O arquivo processos.txt

Cada linha representa um processo com 4 números:

```
1 10 200 2
↑  ↑  ↑  ↑
│  │  │  └── Prioridade
│  │  └───── Tamanho na RAM (bytes)
│  └──────── Burst Time — tempo de CPU necessário (segundos)
└─────────── PID — identificador do processo
```

Exemplo completo:
```
1 10 200 2   → Processo 1, precisa de 10s de CPU, ocupa 200B de RAM, prioridade 2
2  4 100 1   → Processo 2, precisa de 4s de CPU, ocupa 100B de RAM, prioridade 1
3  6 512 3   → Processo 3, precisa de 6s de CPU, ocupa 512B de RAM, prioridade 3
4  2 128 2   → Processo 4, precisa de 2s de CPU, ocupa 128B de RAM, prioridade 2
```

---

### Exemplo de saída do programa

```
--------------------------------------------------
Tempo Atual: 04 s
CPU: Executando PID 2 (Restam 2s)
Fila de Prontos: [PID 3] -> [PID 1]
Mapa da RAM: [P1: 200B][P2: 100B][P3: 512B][Livre: 212B]
--------------------------------------------------
```

| Linha | O que mostra |
|-------|-------------|
| `Tempo Atual` | Relógio da simulação em segundos |
| `CPU` | Qual processo está rodando agora e quanto tempo falta |
| `Fila de Prontos` | Próximos processos a usar a CPU, em ordem |
| `Mapa da RAM` | Como a memória está distribuída neste momento |

---

## Tecnologias utilizadas

| Tecnologia | Para que serve neste projeto |
|------------|------------------------------|
| **Linguagem C** | Linguagem de programação usada (próxima ao hardware, ideal para simular SO) |
| **GCC** | Compilador — transforma o código `.c` em programa executável |
| **Makefile** | Automatiza a compilação com o comando `make` |
| **Git** | Controle de versão — salva o histórico de mudanças no código |
| **GitHub** | Plataforma online para guardar e compartilhar o código |

---

## Autores

Projeto desenvolvido em dupla para a disciplina de Infraestrutura de Software.  
Profa. Monique Soares — Apresentação: 01/06/2026
