CC = gcc
CFLAGS = -Wall
EXEC = ll

SRC_PATH = src/
HEADER_PATH = header/
BIN_PATH = bin/

all: $(BIN_PATH)$(EXEC)

ll: $(BIN_PATH)ll.o
	$(CC) -o $@ $^

$(BIN_PATH)ll.o: $(HEADER_PATH)ll.h

$(BIN_PATH)%.o: $(SRC_PATH)%.c
	$(CC) -o $@ -c $< $(CFLAGS)

run:
	./$(BIN_PATH)$(EXEC)

clean:
	rm -rf $(BIN_PATH)*.o

tar: mrproper
	tar cf $(EXEC).tar README.txt Makefile bin/ src/ header/ INSTALL.sh REMOVE.sh resources/
mrproper: clean
	rm -rf $(BIN_PATH)$(EXEC)
