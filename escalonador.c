/* =====================================================
   escalonador.c  –  Escalonamento Round-Robin
   Infraestrutura de Software – Projeto 2°GQ
   ===================================================== */

#include <stdio.h>
#include "escalonador.h"
#include "memoria.h"

/* ---- Definição das filas globais ------------------- */

PCB *fila_prontos[MAX_PROCESSOS];
int  fila_prontos_inicio   = 0;
int  fila_prontos_fim      = 0;
int  fila_prontos_tamanho  = 0;

PCB *fila_espera[MAX_PROCESSOS];
int  fila_espera_tamanho   = 0;

/* ==================================================
   FILA DE PRONTOS  (fila circular)
   ================================================== */

/* Adiciona processo no final da fila de prontos */
void fila_prontos_enfileirar(PCB *p) {
    if (fila_prontos_tamanho >= MAX_PROCESSOS) {
        printf("[ERRO] Fila de prontos cheia!\n");
        return;
    }
    p->estado = PRONTO;
    fila_prontos[fila_prontos_fim] = p;
    fila_prontos_fim = (fila_prontos_fim + 1) % MAX_PROCESSOS;
    fila_prontos_tamanho++;
}

/* Remove e retorna o processo do início da fila */
PCB *fila_prontos_desenfileirar(void) {
    if (fila_prontos_tamanho == 0) return NULL;

    PCB *p = fila_prontos[fila_prontos_inicio];
    fila_prontos_inicio = (fila_prontos_inicio + 1) % MAX_PROCESSOS;
    fila_prontos_tamanho--;
    return p;
}

/* Retorna 1 se a fila de prontos estiver vazia */
int fila_prontos_vazia(void) {
    return fila_prontos_tamanho == 0;
}

/* ==================================================
   FILA DE ESPERA  (array simples)
   ================================================== */

/* Adiciona processo na fila de espera (sem RAM) */
void fila_espera_adicionar(PCB *p) {
    if (fila_espera_tamanho >= MAX_PROCESSOS) {
        printf("[ERRO] Fila de espera cheia!\n");
        return;
    }
    p->estado = ESPERANDO;
    fila_espera[fila_espera_tamanho++] = p;
    printf("  >> PID %d aguardando memoria (%dB necessarios, %dB livres)\n",
           p->pid, p->tamanho_ram, memoria_bytes_livres());
}

/* Tenta mover processos da fila de espera para a fila
   de prontos quando há memória disponível.            */
void fila_espera_tentar_admitir(void) {
    int i = 0;
    while (i < fila_espera_tamanho) {
        PCB *p = fila_espera[i];
        int base = memoria_alocar(p);

        if (base != -1) {
            /* Conseguiu memória: vai para prontos */
            printf("  >> PID %d admitido na RAM (base=%d)\n", p->pid, base);
            fila_prontos_enfileirar(p);

            /* Remove da fila de espera compactando */
            for (int j = i; j < fila_espera_tamanho - 1; j++) {
                fila_espera[j] = fila_espera[j + 1];
            }
            fila_espera_tamanho--;
            /* Não incrementa i: reavalia a mesma posição */
        } else {
            i++;
        }
    }
}

/* ==================================================
   ESCALONADOR  –  Round-Robin
   ================================================== */

/* Executa um ciclo de QUANTUM unidades de tempo para
   o processo no início da fila de prontos.
   
   Fluxo:
   1. Retira o processo da fila.
   2. Marca como EXECUTANDO.
   3. Subtrai o quantum do burst_restante.
   4a. Se burst_restante <= 0: processo ENCERRADO,
       libera memória, tenta admitir novos.
   4b. Se ainda tem tempo: volta para o FIM da fila
       (Round-Robin).
   Retorna o ponteiro do processo executado.          */
PCB *escalonador_executar_ciclo(void) {
    if (fila_prontos_vazia()) return NULL;

    PCB *p = fila_prontos_desenfileirar();
    p->estado = EXECUTANDO;

    /* Decrementa pelo quantum (mas não abaixo de 0) */
    int tempo_executado = (p->burst_restante < QUANTUM)
                          ? p->burst_restante
                          : QUANTUM;
    p->burst_restante -= tempo_executado;

    if (p->burst_restante <= 0) {
        /* Processo concluído */
        p->estado = ENCERRADO;
        memoria_liberar(p);
        printf("  >> PID %d ENCERRADO. Memoria liberada.\n", p->pid);

        /* Tenta admitir processos que aguardavam RAM */
        fila_espera_tentar_admitir();
    } else {
        /* Quantum expirado: troca de contexto, volta para o fim */
        printf("  >> PID %d: quantum expirado, volta para fila (restam %ds)\n",
               p->pid, p->burst_restante);
        fila_prontos_enfileirar(p);
    }

    return p;
}

/* Imprime o conteúdo atual da fila de prontos */
void escalonador_imprimir_fila(void) {
    if (fila_prontos_vazia()) {
        printf("Fila de Prontos: [vazia]\n");
        return;
    }

    printf("Fila de Prontos: ");
    for (int i = 0; i < fila_prontos_tamanho; i++) {
        int idx = (fila_prontos_inicio + i) % MAX_PROCESSOS;
        printf("[PID %d]", fila_prontos[idx]->pid);
        if (i < fila_prontos_tamanho - 1) printf(" -> ");
    }
    printf("\n");
}
