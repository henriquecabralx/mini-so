/* =====================================================
   pcb.c  –  Implementação auxiliar do PCB
   Infraestrutura de Software – Projeto 2°GQ
   ===================================================== */

#include "pcb.h"

/* Retorna a representação textual do estado */
const char* estado_para_string(Estado e) {
    switch (e) {
        case NOVO:       return "NOVO";
        case PRONTO:     return "PRONTO";
        case EXECUTANDO: return "EXECUTANDO";
        case ESPERANDO:  return "ESPERANDO";
        case ENCERRADO:  return "ENCERRADO";
        default:         return "DESCONHECIDO";
    }
}
