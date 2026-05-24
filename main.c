/* =====================================================
   main.c  –  Ponto de entrada do Mini-SO
   Infraestrutura de Software – Projeto 2°GQ
   Profa. Monique Soares

   Fluxo geral:
   1. Lê processos.txt (ou entrada manual pelo teclado)
   2. Inicializa memória e fila de prontos
   3. Loop principal: a cada ciclo imprime status,
      tenta admitir processos, executa um quantum
   4. Ao final grava log e encerra
   ===================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcb.h"
#include "memoria.h"
#include "escalonador.h"
#include "log.h"

/* ---- Constantes ------------------------------------ */
#define MAX_PROCESSOS 64
#define ARQUIVO_LOG   "simulacao_log.txt"

/* ---- Variáveis globais ----------------------------- */
static PCB processos[MAX_PROCESSOS]; /* Array com todos os PCBs    */
static int total_processos = 0;      /* Quantidade lida do arquivo */
static int tempo_atual     = 0;      /* Relógio da simulação       */

/* ---- Protótipos locais ----------------------------- */
static void ler_arquivo(const char *caminho);
static void ler_teclado(void);
static void admitir_processos_novos(void);
static void imprimir_status(PCB *em_execucao);
static int  todos_encerrados(void);

/* ==================================================
   MAIN
   ================================================== */
int main(void) {
    char opcao;
    char caminho[256];

    printf("================================================\n");
    printf("   Mini-SO  –  Simulador de Processos e Memoria\n");
    printf("   Infraestrutura de Software  –  2GQ\n");
    printf("   Alunos: Allan Henrique e Luana Larissa");
    printf("================================================\n\n");

    /* Pergunta ao usuário como deseja fornecer os processos */
    printf("Deseja ler de arquivo (A) ou digitar pelo teclado (T)? ");
    scanf(" %c", &opcao);

    if (opcao == 'A' || opcao == 'a') {
        printf("Informe o nome do arquivo (ex: processos.txt): ");
        scanf("%s", caminho);
        ler_arquivo(caminho);
    } else {
        ler_teclado();
    }

    if (total_processos == 0) {
        printf("[ERRO] Nenhum processo carregado. Encerrando.\n");
        return 1;
    }

    printf("\n[INFO] %d processo(s) carregado(s). Quantum = %ds, RAM = %dB\n\n",
           total_processos, QUANTUM, RAM_TAMANHO);

    /* Inicializa subsistemas */
    memoria_inicializar();
    log_inicializar(ARQUIVO_LOG);

    /* Tenta admitir todos os processos na RAM logo no início */
    admitir_processos_novos();

    /* ==================================================
       LOOP PRINCIPAL DA SIMULAÇÃO
       Continua enquanto houver processos não encerrados.
       ================================================== */
    while (!todos_encerrados()) {

        /* Tenta admitir processos que ainda não têm RAM */
        fila_espera_tentar_admitir();

        /* Verifica se há algo para executar */
        if (fila_prontos_vazia()) {
            /* Situação de impasse: nenhum processo na fila
               mas ainda existem processos não encerrados.
               Pode ocorrer se todos estiverem esperando RAM.
               Avança o tempo e tenta de novo.               */
            printf("\n[t=%03ds] CPU ociosa – todos aguardando memoria.\n",
                   tempo_atual);

            char msg[128];
            snprintf(msg, sizeof(msg), "CPU ociosa – todos os processos aguardam RAM.");
            log_evento(tempo_atual, msg);

            tempo_atual += QUANTUM;
            continue;
        }

        /* Obtém o processo que será executado neste ciclo */
        PCB *em_execucao = fila_prontos[fila_prontos_inicio];

        /* Imprime o status ANTES de executar */
        imprimir_status(em_execucao);

        /* Executa um quantum */
        em_execucao = escalonador_executar_ciclo();

        /* Registra evento no log */
        char msg[256];
        if (em_execucao->estado == ENCERRADO) {
            snprintf(msg, sizeof(msg),
                     "PID %d ENCERRADO apos executar no t=%d.",
                     em_execucao->pid, tempo_atual);
        } else {
            snprintf(msg, sizeof(msg),
                     "PID %d executou quantum (%ds). Restam %ds.",
                     em_execucao->pid, QUANTUM, em_execucao->burst_restante);
        }
        log_evento(tempo_atual, msg);

        /* Avança o relógio */
        tempo_atual += QUANTUM;
    }

    /* ==================================================
       ENCERRAMENTO
       ================================================== */
    printf("\n================================================\n");
    printf("   Simulacao concluida! Todos os processos encerrados.\n");
    printf("   Tempo total simulado: %d segundos\n", tempo_atual);
    printf("   Bytes livres na RAM: %d / %d\n",
           memoria_bytes_livres(), RAM_TAMANHO);
    printf("================================================\n\n");

    /* Mapa final da memória */
    memoria_imprimir_mapa();

    /* Salva log */
    log_estado_final_memoria();
    log_resumo_processos(processos, total_processos);
    log_finalizar();

    return 0;
}

/* ==================================================
   LER ARQUIVO
   Formato de cada linha: PID BurstTime RAM Prioridade
   Exemplo: 1 10 200 2
   ================================================== */
static void ler_arquivo(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) {
        printf("[ERRO] Arquivo '%s' nao encontrado.\n", caminho);
        return;
    }

    int pid, burst, ram_size, prio;
    while (fscanf(f, "%d %d %d %d", &pid, &burst, &ram_size, &prio) == 4) {
        if (total_processos >= MAX_PROCESSOS) {
            printf("[AVISO] Limite de %d processos atingido.\n", MAX_PROCESSOS);
            break;
        }
        PCB *p = &processos[total_processos++];
        p->pid            = pid;
        p->burst_total    = burst;
        p->burst_restante = burst;
        p->tamanho_ram    = ram_size;
        p->prioridade     = prio;
        p->base_ram       = -1;
        p->estado         = NOVO;

        printf("  Processo carregado: PID=%d Burst=%ds RAM=%dB Prio=%d\n",
               p->pid, p->burst_total, p->tamanho_ram, p->prioridade);
    }
    fclose(f);
}

/* ==================================================
   LER PELO TECLADO
   ================================================== */
static void ler_teclado(void) {
    int n;
    printf("Quantos processos deseja cadastrar? ");
    scanf("%d", &n);
    if (n > MAX_PROCESSOS) n = MAX_PROCESSOS;

    for (int i = 0; i < n; i++) {
        PCB *p = &processos[total_processos++];
        printf("\n--- Processo %d ---\n", i + 1);

        printf("  PID            : "); scanf("%d", &p->pid);
        printf("  Burst Time (s) : "); scanf("%d", &p->burst_total);
        printf("  Tamanho RAM (B): "); scanf("%d", &p->tamanho_ram);
        printf("  Prioridade     : "); scanf("%d", &p->prioridade);

        p->burst_restante = p->burst_total;
        p->base_ram       = -1;
        p->estado         = NOVO;
    }
}

/* ==================================================
   ADMITIR PROCESSOS NOVOS
   Percorre o array de processos e tenta alocar
   na RAM todos que estejam no estado NOVO.
   ================================================== */
static void admitir_processos_novos(void) {
    for (int i = 0; i < total_processos; i++) {
        PCB *p = &processos[i];
        if (p->estado != NOVO) continue;

        int base = memoria_alocar(p);
        if (base != -1) {
            printf("[ADMISSAO] PID %d alocado na RAM (base=%d, %dB)\n",
                   p->pid, base, p->tamanho_ram);
            fila_prontos_enfileirar(p);
        } else {
            printf("[ADMISSAO] PID %d sem memoria (%dB necessarios, %dB livres)\n",
                   p->pid, p->tamanho_ram, memoria_bytes_livres());
            fila_espera_adicionar(p);
        }
    }
}

/* ==================================================
   IMPRIMIR STATUS (formato pedido no projeto)

   --------------------------------------------------
   Tempo Atual: 04 s
   CPU: Executando PID 2 (Restam 2s)
   Fila de Prontos: [PID 3] -> [PID 1]
   Mapa da RAM: [P1: 200B][P2: 100B][Livre: 724B]
   --------------------------------------------------
   ================================================== */
static void imprimir_status(PCB *em_execucao) {
    printf("\n--------------------------------------------------\n");
    printf("Tempo Atual: %02d s\n", tempo_atual);

    if (em_execucao) {
        printf("CPU: Executando PID %d (Restam %ds)\n",
               em_execucao->pid, em_execucao->burst_restante);
    } else {
        printf("CPU: Ociosa\n");
    }

    escalonador_imprimir_fila();
    memoria_imprimir_mapa();

    /* Fila de espera (se houver) */
    if (fila_espera_tamanho > 0) {
        printf("Fila de Espera (sem RAM): ");
        for (int i = 0; i < fila_espera_tamanho; i++) {
            printf("[PID %d]", fila_espera[i]->pid);
            if (i < fila_espera_tamanho - 1) printf(" -> ");
        }
        printf("\n");
    }

    printf("--------------------------------------------------\n");
}

/* ==================================================
   TODOS ENCERRADOS
   Retorna 1 se todos os processos estiverem no
   estado ENCERRADO.
   ================================================== */
static int todos_encerrados(void) {
    for (int i = 0; i < total_processos; i++) {
        if (processos[i].estado != ENCERRADO) return 0;
    }
    return 1;
}
