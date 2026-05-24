#ifndef MEMORIA_H
#define MEMORIA_H

/* =====================================================
   memoria.h  –  Gerência de Memória RAM (First-Fit)
   Infraestrutura de Software – Projeto 2°GQ
   ===================================================== */

#include "pcb.h"

#define RAM_TAMANHO 1024   /* 1 KB de RAM simulada */

/* RAM representada como array de inteiros:
   0  = posição livre
   N  = posição ocupada pelo processo de PID N           */
extern int ram[RAM_TAMANHO];

/* Inicializa toda a RAM com 0 (livre) */
void memoria_inicializar(void);

/* First-Fit: encontra o primeiro bloco contíguo livre
   com tamanho >= p->tamanho_ram.
   Retorna endereço base ou -1 se não couber.           */
int memoria_alocar(PCB *p);

/* Libera todas as posições ocupadas pelo processo p.   */
void memoria_liberar(PCB *p);

/* Imprime mapa compacto da RAM no console.             */
void memoria_imprimir_mapa(void);

/* Retorna quantidade de bytes livres na RAM.           */
int memoria_bytes_livres(void);

#endif /* MEMORIA_H */
