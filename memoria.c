/* =====================================================
   memoria.c  –  Gerência de Memória RAM (First-Fit)
   Infraestrutura de Software – Projeto 2°GQ
   ===================================================== */

#include <stdio.h>
#include "memoria.h"

/* Declaração global da RAM: 0 = livre, PID = ocupado */
int ram[RAM_TAMANHO];

/* --------------------------------------------------
   memoria_inicializar
   Zera todo o array, marcando tudo como livre.
   -------------------------------------------------- */
void memoria_inicializar(void) {
    for (int i = 0; i < RAM_TAMANHO; i++) {
        ram[i] = 0;
    }
}

/* --------------------------------------------------
   memoria_alocar  (algoritmo First-Fit)
   Percorre a RAM procurando o primeiro bloco
   contíguo com tamanho >= p->tamanho_ram.
   Se encontrar, marca as posições com o PID e
   guarda o endereço base no campo p->base_ram.
   Retorna o endereço base ou -1 se não couber.
   -------------------------------------------------- */
int memoria_alocar(PCB *p) {
    int inicio = -1;
    int contador = 0;

    for (int i = 0; i < RAM_TAMANHO; i++) {
        if (ram[i] == 0) {
            /* Posição livre: começa ou continua bloco */
            if (inicio == -1) inicio = i;
            contador++;

            if (contador == p->tamanho_ram) {
                /* Bloco suficiente encontrado: aloca */
                for (int j = inicio; j < inicio + p->tamanho_ram; j++) {
                    ram[j] = p->pid;
                }
                p->base_ram = inicio;
                return inicio;
            }
        } else {
            /* Posição ocupada: reinicia busca */
            inicio   = -1;
            contador = 0;
        }
    }

    /* Não encontrou espaço suficiente */
    return -1;
}

/* --------------------------------------------------
   memoria_liberar
   Percorre a RAM e zera todas as posições cujo
   valor seja igual ao PID do processo.
   -------------------------------------------------- */
void memoria_liberar(PCB *p) {
    for (int i = 0; i < RAM_TAMANHO; i++) {
        if (ram[i] == p->pid) {
            ram[i] = 0;
        }
    }
    p->base_ram = -1;
}

/* --------------------------------------------------
   memoria_imprimir_mapa
   Gera um mapa compacto agrupando regiões
   consecutivas do mesmo processo (ou livres).
   Exemplo: [P1: 200B][P2: 100B][Livre: 724B]
   -------------------------------------------------- */
void memoria_imprimir_mapa(void) {
    printf("Mapa da RAM: ");

    int i = 0;
    while (i < RAM_TAMANHO) {
        int pid_atual = ram[i];
        int inicio    = i;

        /* Conta o tamanho do bloco atual */
        while (i < RAM_TAMANHO && ram[i] == pid_atual) {
            i++;
        }
        int tamanho = i - inicio;

        if (pid_atual == 0) {
            printf("[Livre: %dB]", tamanho);
        } else {
            printf("[P%d: %dB]", pid_atual, tamanho);
        }
    }
    printf("\n");
}

/* --------------------------------------------------
   memoria_bytes_livres
   Conta quantas posições (bytes) estão livres.
   -------------------------------------------------- */
int memoria_bytes_livres(void) {
    int livres = 0;
    for (int i = 0; i < RAM_TAMANHO; i++) {
        if (ram[i] == 0) livres++;
    }
    return livres;
}
