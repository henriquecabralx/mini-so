#ifndef PCB_H
#define PCB_H

/* =====================================================
   pcb.h  –  Definição da estrutura do Processo (PCB)
   Infraestrutura de Software – Projeto 2°GQ
   ===================================================== */

/* Estados possíveis de um processo */
typedef enum {
    NOVO,
    PRONTO,
    EXECUTANDO,
    ESPERANDO,   /* aguardando memória */
    ENCERRADO
} Estado;

/* Bloco de Controle de Processo (PCB) */
typedef struct {
    int   pid;            /* Identificador do processo          */
    int   burst_total;    /* Tempo de execução total (s)        */
    int   burst_restante; /* Tempo que ainda falta executar     */
    int   tamanho_ram;    /* Espaço necessário em RAM (bytes)   */
    int   prioridade;     /* Prioridade (menor = mais urgente)  */
    int   base_ram;       /* Endereço inicial na RAM (-1 = fora)*/
    Estado estado;        /* Estado atual do processo           */
} PCB;

/* Converte enum Estado em string legível */
const char* estado_para_string(Estado e);

#endif /* PCB_H */
