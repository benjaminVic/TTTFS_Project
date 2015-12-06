CC = gcc
CFLAGS = -Wall

SRC_PATH = src/
HEADER_PATH = header/
BIN_PATH = bin/

all: tfs_create tfs_partition tfs_analyze tfs_format libll libtfs

tfs_create: $(BIN_PATH)tfs_create.o
	$(CC) -o $(BIN_PATH)$@ $^

tfs_partition: $(BIN_PATH)tfs_partition.o $(BIN_PATH)ll.o
	$(CC) -o $(BIN_PATH)$@ $^

tfs_analyze: $(BIN_PATH)tfs_analyze.o $(BIN_PATH)ll.o
	$(CC) -o $(BIN_PATH)$@ $^

tfs_format: $(BIN_PATH)tfs_format.o $(BIN_PATH)tfs.o
	$(CC) -o $(BIN_PATH)$@ $^

libll: $(BIN_PATH)ll.o
	$(CC) -o $(BIN_PATH)$@.so -shared -fPIC $^

libtfs: $(BIN_PATH)tfs.o
	$(CC) -o $(BIN_PATH)$@.so -shared -fPIC $^

tfs_create.o: $(HEADER_PATH)tfs_create.h
tfs_partition.o: $(HEADER_PATH)tfs_partition.h
tfs_analyze.o: $(HEADER_PATH)tfs_analyze.h
tfs_format.o: $(HEADER_PATH)tfs_format.h
ll.o: $(HEADER_PATH)ll.h
tfs.o: $(HEADER_PATH)tfs.h

$(BIN_PATH)%.o: $(SRC_PATH)%.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(BIN_PATH)*.o

tar: mrproper
	tar cf TTTFS_Project.tar Makefile bin/ src/ header/ _INSTALL.sh _REMOVE.sh
	
mrproper: clean
	rm -rf $(BIN_PATH)tfs_create $(BIN_PATH)tfs_partition $(BIN_PATH)tfs_analyze $(BIN_PATH)tfs_format $(BIN_PATH)libll.so $(BIN_PATH)libtfs.so

install:
	sudo ./_INSTALL.sh

remove:
	sudo ./_REMOVE.sh
