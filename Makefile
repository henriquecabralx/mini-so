# =====================================================
#  Makefile  –  Mini-SO
#  Infraestrutura de Software – Projeto 2°GQ
# =====================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g
TARGET  = mini_so
SRCS    = main.c pcb.c memoria.c escalonador.c log.c
OBJS    = $(SRCS:.c=.o)

# Regra padrão
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Compilacao bem-sucedida! Execute com: ./$(TARGET)"

# Compilação de cada .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpa arquivos gerados
clean:
	rm -f $(OBJS) $(TARGET) simulacao_log.txt

# Executa com o arquivo padrão de processos
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
