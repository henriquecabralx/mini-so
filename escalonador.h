#ifndef ESCALONADOR_H
#define ESCALONADOR_H

/* =====================================================
   escalonador.h  –  Escalonamento Round-Robin
   Infraestrutura de Software – Projeto 2°GQ
   ===================================================== */

#include "pcb.h"

#define QUANTUM       2    /* Quantum fixo em segundos         */
#define MAX_PROCESSOS 64   /* Máximo de processos suportados   */

/* ---- Filas ----------------------------------------- */

/* Fila de Prontos: processos aguardando CPU (circular)  */
extern PCB *fila_prontos[MAX_PROCESSOS];
extern int  fila_prontos_inicio;
extern int  fila_prontos_fim;
extern int  fila_prontos_tamanho;

/* Fila de Espera: processos sem memória disponível      */
extern PCB *fila_espera[MAX_PROCESSOS];
extern int  fila_espera_tamanho;

/* ---- Operações das filas --------------------------- */
void    fila_prontos_enfileirar(PCB *p);
PCB    *fila_prontos_desenfileirar(void);
int     fila_prontos_vazia(void);

void    fila_espera_adicionar(PCB *p);
void    fila_espera_tentar_admitir(void); /* tenta mover para prontos */

/* ---- Escalonador ----------------------------------- */

/* Executa um ciclo de quantum para o processo no topo.
   Retorna o ponteiro do processo executado (ou NULL).   */
PCB *escalonador_executar_ciclo(void);

/* Imprime o estado da fila de prontos no console.       */
void escalonador_imprimir_fila(void);

#endif /* ESCALONADOR_H */
