#ifndef LOG_H
#define LOG_H

/* =====================================================
   log.h  –  Módulo de Log (Opção B do Módulo 3)
   Infraestrutura de Software – Projeto 2°GQ
   ===================================================== */

#include "pcb.h"

/* Abre (ou cria) o arquivo de log para escrita.        */
void log_inicializar(const char *nome_arquivo);

/* Registra uma linha de evento no arquivo de log.      */
void log_evento(int tempo, const char *mensagem);

/* Grava o estado final da memória RAM no log.          */
void log_estado_final_memoria(void);

/* Grava o resumo final de todos os processos.          */
void log_resumo_processos(PCB *processos, int total);

/* Fecha o arquivo de log.                              */
void log_finalizar(void);

#endif /* LOG_H */
