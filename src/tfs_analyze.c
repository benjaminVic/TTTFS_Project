#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../header/tfs_analyze.h"
#include "../header/ll.h"

int main(int argc, char *argv[]){

	uint32_t number_of_blocks = 0;
	uint32_t number_of_partitions = 0;
	uint32_t size_of_disk = 0;
	uint32_t size_of_partition = 0;
	block b;

	// Test la présence du bon nombre d'arguments
	if(argc != 2){
		printf("%s\n", "Usage: tfs_analyze [name]");
		exit(1);
	}

	// On monte le disque en lui donnant l'id 0
	error start = start_disk(argv[1], 0); 
	// Si le disque n'est pas monté
	if(start != 0){
		fprintf(stderr, "Erreur %d: %s\n", start, strError(start));
		exit(1);
	}

	// On tente de lire le block 0
	error read = read_block(0, b, 0);
	// Si la lecture échoue
	if(read != 0){
		fprintf(stderr, "Erreur %d: %s\n", read, strError(read));
		stop_disk(0);
		exit(1);
	}

	// Lecture du nombre de blocks
	number_of_blocks = readBlockToInt(b, 0);
	size_of_disk = (number_of_blocks * 1024) / 1000;
	// Lecture du nombre de partitions
	number_of_partitions = readBlockToInt(b, 1);

	// Affichage des informations
	printf("%s\n", "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
	printf("%s : %s\n", " # Disk", argv[1]);
	printf("%s : %d blocks | %dko\n", " -> Size", number_of_blocks, size_of_disk);
	printf("%s : %d\n", " -> Partitions", number_of_partitions);
	for(int i=0; i<number_of_partitions; i++){
		size_of_partition = readBlockToInt(b, i+2);
		printf("%s %d = %d blocks | %dko\n", "   -", i, size_of_partition, (size_of_partition * 1024) / 1000);
	}
	printf("%s\n", "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");

	stop_disk(0);
	return 0;
}
