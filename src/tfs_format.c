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

	uint32_t first_partition_blck = 0;
	uint32_t size_of_partition = 0;
	uint32_t size_of_table = 0;
	uint32_t total_size = 0;

	PARTITION_INFO infPartition;

	// Vérification de l'existance du disque
	if(access(disk_name, F_OK) != 0){
		return _DISK_NOT_FOUND;
	}
	// On monte le disque en lui donnant l'id 0
	error start = start_disk(disk_name, 0); 
	if(start != 0){
		return start;
	}

	// On récupère le numero du premier block de cette partition et la taille
	error sizePartition = getPartitionSize(0, partition, &size_of_partition);
	if(sizePartition != 0){
		stop_disk(0);
		return sizePartition;
	}

	error fst_blck = getFirstPartitionBlck(0, partition, &first_partition_blck);
	if(fst_blck != 0){
		stop_disk(0);
		return fst_blck;
	}

	// Calcul de la taille nécessaire pour la table des fichiers (64o par entrée)
	// Dans un block de de la table on peut mettre 1024/64 = 16 fichiers
	if((file_count % 16) == 0)
		size_of_table = (file_count / 16);
	else
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

	// Confirmation du formatage
	char validation[16];
	printf("%s", "Le formatage entraine la perte de données, êtes-vous sûr ?(y/n) ");
	scanf("%s", validation);

	if( (strcmp(validation, "Y") == 0) || (strcmp(validation, "y") == 0) ){
		// On efface la partition (de first_partition_blck à size_of_partition+1)
		error erase_disk = eraseDisk(0, first_partition_blck, size_of_partition+1);
		if(erase_disk != 0)
			return erase_disk;
	}
	else{
		stop_disk(0);
		exit(1);
	}

	// Modification des informations
	infPartition.TTTFS_MAGIC_NUMBER = MAGIC_NUMBER;
	infPartition.TTTFS_VOLUME_BLOCK_SIZE = BLCK_SIZE;
	infPartition.TTTFS_VOLUME_BLOCK_COUNT = size_of_partition;
	infPartition.TTTFS_VOLUME_FREE_BLOCK_COUNT = size_of_partition - size_of_table - 1; // nombre de blocs - (description + ceux de la table)
	infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK = size_of_table + 1; // (Numero de bloc DISK donc + 1 pour sauter les informations du DISK)
	infPartition.TTTFS_VOLUME_MAX_FILE_COUNT = file_count; 
	infPartition.TTTFS_VOLUME_FREE_FILE_COUNT = file_count;
	infPartition.TTTFS_VOLUME_FIRST_FREE_FILE = 0;

	// On écrit les données dans le premier block de la partition (Description block)
	error write_desc_block = writePartitionInfos(0, infPartition, partition);
	if(write_desc_block != 0){
		stop_disk(0);
		return write_desc_block;
	}

	printInfoPartition(infPartition);

	//______________________________________________________________________________
	// Initialisation de la table des fichiers et du chainage des blocs libres
	initFilesTable(0, partition);
	initFreeBlockChain(0, partition);
	//______________________________________________________________________________

	// Création de l'entrée racine
	FILE_ENTRY racine; 
	initFileEntry(&racine);
	setTypeFile(&racine, 1);
	setSubTypeFile(&racine, 0);
	//addDirectBlock(&racine, size_of_table+1);

	// Ajout de l'entrée racine dans la table de la partition
	addFileEntryToTable(0, partition, racine);


	// ##################### TESTS ########################
	FILE_ENTRY testEntry; 
	initFileEntry(&testEntry);

	addFileEntryToTable(0, partition, testEntry);

	// Suppression de l'entrée 0 de la table de la partition
	//removeFileEntryInTable(0, partition, 1);

 	
	// Récupération des informations
	error read_infos_part = readPartitionInfos(0, &infPartition, partition);
	if(read_infos_part != 0)
		return read_infos_part;
	
	//printInfoPartition(infPartition);

	readFileEntryFromTable(0, partition, &racine, 0);
	readFileEntryFromTable(0, partition, &testEntry, 1);

	//printFileEntry(racine);
	//printFileEntry(testEntry);
	// ##############################################################

/*
	// Création des entrées de répertoire "." et ".." de la racine
	DIR_ENTRY dot;
	DIR_ENTRY double_dot;

	dot.number = 0;
	dot.name[0] = '.';
	dot.name[1] = '\0';

	double_dot.number = 0;
	double_dot.name[0] = '.';
	double_dot.name[1] = '.';
	double_dot.name[2] = '\0';

	block b_racine;
	// On tente de lire le block concernant le fichier 0 (la racine)
	read_block(0, b_racine, infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK);

	// Ecriture dans le block
	writeDirEntryToBlock(b_racine, 0, dot);
	writeDirEntryToBlock(b_racine, 1, double_dot);
	// Ecriture du block sur le disque
	write_block(0, b_racine, infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK);

	//printBlock(b_racine);

	//########## TEST LECTURE DIR ENTRY ############
	DIR_ENTRY testEntry;
	read_block(0, b_racine, infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK);
	readBlockToDirEntry(b_racine, 1, &testEntry);
	printf("%s\n", testEntry.name);
	//##############################################
*/

	stop_disk(0);
	return _NOERROR;
}
