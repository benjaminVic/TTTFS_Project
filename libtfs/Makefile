CC = gcc
CFLAGS = -Wall
EXEC = libtfs

SRC_PATH = src/
HEADER_PATH = header/
BIN_PATH = bin/

all: $(BIN_PATH)$(EXEC)

libtfs: $(BIN_PATH)libtfs.o
	$(CC) -o $@ $^

$(BIN_PATH)libtfs.o: $(HEADER_PATH)libtfs.h

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
