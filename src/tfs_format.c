#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../header/tfs_format.h"

int main(int argc, char *argv[]){
	int num_partition;
	int file_count;
	char* disk_name;

	// Test la présence du bon nombre d'arguments
	if(argc != 6){
		printf("%s\n", "Usage: tfs_format -p partition -mf file_count [disk]");
		exit(1);
	}

	// Récupération des tailles
	for(int i=0; i<argc; i++){
 		if(strcmp(argv[i], "-p") == 0)
 			num_partition = atoi(argv[i+1]);
 		if(strcmp(argv[i], "-mf") == 0)
 		{
 			file_count = atoi(argv[i+1]);
 		}
 	}

 	disk_name = argv[argc-1];
 	error init = init_partition(disk_name, num_partition, file_count);

 	if(init != 0){
 		fprintf(stderr, "Erreur %d: %s\n", init, strError(init));
 		exit(1);
 	}
	return 0;
}

error init_partition(char* disk_name, int partition, uint32_t file_count){
	uint32_t number_of_partitions = 0;
	uint32_t size_of_partition = 0;
	uint32_t size_of_table = 0;
	uint32_t total_size = 0;
	uint32_t first_partition_blck = 1;

	block infosDisk;
	block infosPartition;

	// Vérification de l'existance du disque
	if(access(disk_name, F_OK) != 0){
		return _DISK_NOT_FOUND;
	}
	
	// On monte le disque en lui donnant l'id 0
	error start = start_disk(disk_name, 0); 
	if(start != 0){
		return start;
	}

	// On tente de lire le block 0
	error read_infos_disk = read_block(0, infosDisk, 0);
	if(read_infos_disk != 0){
		stop_disk(0);
		return read_infos_disk;
	}

	// Lecture du nombre de partitions
	number_of_partitions = readBlockToInt(infosDisk, 1);
	// Vérification de l'existence de la partition
	if(partition >= number_of_partitions)
		return _PARTITION_NOT_FOUND;

	// Récupération des tailles des partitions
	uint32_t partitionsSizes[number_of_partitions];
	//(+2 pour arriver aux entiers des tailles de partition présentes)
	for(int i=0; i<number_of_partitions; i++){
		partitionsSizes[i] = readBlockToInt(infosDisk, i+2);
	}

	size_of_partition = partitionsSizes[partition];
	// Calcul du premier block de la partition
	for(int i=0; i<partition; i++){
		first_partition_blck += partitionsSizes[i];
	}

	// Calcul de la taille nécessaire pour la table des fichiers (64o par entrée)
	// Dans un block de de la table on peut mettre 1024/64 = 16 fichiers
	// Il faut donc (file_count / 16) + 1 blocks pour la table 
	size_of_table = (file_count / 16) + 1;
	// Calcul de la taille nécessaire total pour la partition
	total_size = 1 + size_of_table + file_count;

	printf("Premier block de p%d = %d\n", partition, first_partition_blck);
	printf("Nombre de fichiers = %d\n", file_count);
	printf("Taille de la table = %d\n", size_of_table);
	printf("Total nécessaire = %d | Partition size = %d\n", total_size, size_of_partition);

	// Vérification de la place disponible
	if(total_size > size_of_partition)
		return _MAX_FILES_TOO_BIG;

	// On récupère le premier block de la partition
	error read_desc_block = read_block(0, infosPartition, first_partition_blck);
	if(read_desc_block != 0){
		stop_disk(0);
		return read_desc_block;
	}

	// Confirmation du formatage
	char validation[16];
	printf("%s", "Le formatage entraine la perte de données, êtes-vous sûr ?(y/n) ");
	scanf("%s", validation);

	if( (strcmp(validation, "Y") == 0) || (strcmp(validation, "y") == 0) ){
		// On remet à zero le premier block de la partition
		error erase_blck = eraseBlock(infosPartition, 0, 256); 
		if(erase_blck != 0)
			fprintf(stderr, "Erreur %d: %s\n", erase_blck, strError(erase_blck));
		// On efface la partition (de first_partition_blck à size_of_partition+1)
		error erase_disk = eraseDisk(0, first_partition_blck, size_of_partition+1);
		if(erase_disk != 0)
			fprintf(stderr, "Erreur %d: %s\n", erase_disk, strError(erase_disk));
	}
	else{
		stop_disk(0);
		exit(1);
	}

	// On écrit les données dans le premier block de la partition (Description block)
	writeIntToBlock(infosPartition, 0, TTTFS_MAGIC_NUMBER);
	writeIntToBlock(infosPartition, 1, TTTFS_VOLUME_BLOCK_SIZE);
	writeIntToBlock(infosPartition, 2, TTTFS_VOLUME_BLOCK_COUNT(size_of_partition));
	writeIntToBlock(infosPartition, 3, TTTFS_VOLUME_FREE_BLOCK_COUNT(file_count));
	writeIntToBlock(infosPartition, 4, TTTFS_VOLUME_FIRST_FREE_BLOCK(size_of_table));
	writeIntToBlock(infosPartition, 5, TTTFS_VOLUME_MAX_FILE_COUNT(file_count));
	writeIntToBlock(infosPartition, 6, TTTFS_VOLUME_FREE_FILE_COUNT(file_count));
	writeIntToBlock(infosPartition, 7, TTTFS_VOLUME_FIRST_FREE_FILE(1)); // Le 0 sera déjà pris pour la racine

	//printBlock(infosPartition);

	// Ecriture du Description block
	error write_desc_block = write_block(0, infosPartition, first_partition_blck);
	if(write_desc_block != 0){
		fprintf(stderr, "Erreur %d: %s\n", write_desc_block, strError(write_desc_block));
		stop_disk(0);
		exit(1);
	}

	stop_disk(0);

	return _NOERROR;
}
