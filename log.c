/* =====================================================
   log.c  –  Módulo de Log em arquivo (Módulo 3 - Opção B)
   Infraestrutura de Software – Projeto 2°GQ
   ===================================================== */

#include <stdio.h>
#include <time.h>
#include "log.h"
#include "memoria.h"

/* Ponteiro para o arquivo de log */
static FILE *arquivo_log = NULL;

/* --------------------------------------------------
   log_inicializar
   Abre o arquivo para escrita e escreve cabeçalho.
   -------------------------------------------------- */
void log_inicializar(const char *nome_arquivo) {
    arquivo_log = fopen(nome_arquivo, "w");
    if (!arquivo_log) {
        printf("[AVISO] Nao foi possivel criar o arquivo de log.\n");
        return;
    }

    /* Cabeçalho com data/hora da simulação */
    time_t agora = time(NULL);
    char *data_str = ctime(&agora);

    fprintf(arquivo_log,
            "=======================================================\n"
            "  Mini-SO  –  Log da Simulacao\n"
            "  Infraestrutura de Software – Projeto 2GQ\n"
            "  Iniciado em: %s"
            "=======================================================\n\n",
            data_str);

    printf("[LOG] Arquivo de log criado: %s\n", nome_arquivo);
}

/* --------------------------------------------------
   log_evento
   Registra um evento com o tempo atual no log.
   -------------------------------------------------- */
void log_evento(int tempo, const char *mensagem) {
    if (!arquivo_log) return;
    fprintf(arquivo_log, "[t=%03ds] %s\n", tempo, mensagem);
}

/* --------------------------------------------------
   log_estado_final_memoria
   Grava o mapa compacto da RAM no final da simulação.
   -------------------------------------------------- */
void log_estado_final_memoria(void) {
    if (!arquivo_log) return;

    fprintf(arquivo_log,
            "\n=======================================================\n"
            "  ESTADO FINAL DA MEMORIA RAM (%d bytes)\n"
            "=======================================================\n",
            RAM_TAMANHO);

    int i = 0;
    while (i < RAM_TAMANHO) {
        int pid_atual = ram[i];
        int inicio    = i;

        while (i < RAM_TAMANHO && ram[i] == pid_atual) i++;
        int tamanho = i - inicio;

        if (pid_atual == 0) {
            fprintf(arquivo_log, "  [Livre    | base=%4d | tamanho=%4dB]\n",
                    inicio, tamanho);
        } else {
            fprintf(arquivo_log, "  [PID %-4d | base=%4d | tamanho=%4dB]\n",
                    pid_atual, inicio, tamanho);
        }
    }

    fprintf(arquivo_log, "\n  Bytes livres  : %d\n", memoria_bytes_livres());
    fprintf(arquivo_log,   "  Bytes ocupados: %d\n",
            RAM_TAMANHO - memoria_bytes_livres());
}

/* --------------------------------------------------
   log_resumo_processos
   Grava tabela com todos os processos e status final.
   -------------------------------------------------- */
void log_resumo_processos(PCB *processos, int total) {
    if (!arquivo_log) return;

    fprintf(arquivo_log,
            "\n=======================================================\n"
            "  RESUMO DOS PROCESSOS\n"
            "=======================================================\n"
            "  %-6s %-10s %-10s %-10s %-12s\n",
            "PID", "Burst Tot", "Burst Res", "RAM(B)", "Estado");
    fprintf(arquivo_log,
            "  ------ ---------- ---------- ---------- ------------\n");

    for (int i = 0; i < total; i++) {
        fprintf(arquivo_log,
                "  %-6d %-10d %-10d %-10d %-12s\n",
                processos[i].pid,
                processos[i].burst_total,
                processos[i].burst_restante,
                processos[i].tamanho_ram,
                estado_para_string(processos[i].estado));
    }
}

/* --------------------------------------------------
   log_finalizar
   Fecha o arquivo de log.
   -------------------------------------------------- */
void log_finalizar(void) {
    if (!arquivo_log) return;

    fprintf(arquivo_log,
            "\n=======================================================\n"
            "  Simulacao encerrada.\n"
            "=======================================================\n");
    fclose(arquivo_log);
    arquivo_log = NULL;
    printf("[LOG] Log salvo com sucesso.\n");
}
