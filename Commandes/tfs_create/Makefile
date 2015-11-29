CC = gcc
CFLAGS = -Wall
EXEC = tfs_create

SRC_PATH = src/
HEADER_PATH = header/
BIN_PATH = bin/

all: $(BIN_PATH)$(EXEC)

tfs_create: $(BIN_PATH)tfs_create.o
	$(CC) -o $@ $^

$(BIN_PATH)tfs_create.o: $(HEADER_PATH)tfs_create.h

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
