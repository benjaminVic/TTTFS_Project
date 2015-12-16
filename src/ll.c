#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../header/ll.h"

// Prototype des fonctions internes 
//*****************************************************************************************
// ATTENTION : Pour le moment ces deux fonctions font le travail exacte de read_block et write_block
// La différence résidera dans le fait qu'elles devront lire ou écrire un paquet de block, stockés dans un buffeur
// pour accroitre les performances. Les deux autres, read_block et write_block, se contenteront de lire ou
// ecrire dans le buffeur puis de le flush (avec l'appel à write_physical) quand il est plein. 
error read_physical_block(disk_id id, block b, uint32_t num);
error write_physical_block(disk_id id, block b, uint32_t num);
//*****************************************************************************************


// Emplacements disponibles pour les disques
DISK _disks[MAX_OPEN_DISK] = {};

//_____________________________________________________________
// Fonctions internes non visibles de l'extérieurs (Pas de signatures dans le .h)
error read_physical_block(disk_id id, block b, uint32_t num){

	// Si le disque n'est pas monté
	if((_disks[id].flag & _MOUNTED) == 0)
		return _DISK_UNMOUNTED;
	// Si le block voulu dépasse le nombre de blocks du disque
	if(!(num < _disks[id].nb_blocks))
		return _NUM_BLCK_TOO_BIG;
	
	// On déplace le curseur a la position du block N (num) : num * BLCK_SIZE
	fseek(_disks[id].disk_descriptor, num * BLCK_SIZE, SEEK_SET);
	// On lit les 1024 prochains caractères, ce qui correspond au bloc N
	if((fread(b, BLCK_SIZE, 1, _disks[id].disk_descriptor)) == -1)
		return _READ_ERROR;

	// Remise à zero de la position du curseur
	rewind(_disks[id].disk_descriptor);
	blockToLtleIndian(b); // Repassage en little indian pour revenir en big indian (plus simple à manipuler)

	return _NOERROR;
}

error write_physical_block(disk_id id, block b, uint32_t num){

	// Si le disque n'est pas monté
	if((_disks[id].flag & _MOUNTED) == 0)
		return _DISK_UNMOUNTED;
	// Si le block voulu dépasse le nombre de blocks du disque
	if(!(num < _disks[id].nb_blocks))
		return _NUM_BLCK_TOO_BIG;

	// On déplace le curseur a la position du block N (num) : num * BLCK_SIZE
	fseek(_disks[id].disk_descriptor, num * BLCK_SIZE, SEEK_SET);

	blockToLtleIndian(b); // Passage en little indian

	// On écrit les 1024 prochains caractères, ce qui correspond au bloc N
	if((fwrite(b, BLCK_SIZE, 1, _disks[id].disk_descriptor)) == -1)
		return _WRITE_ERROR;

	// Remise à zero de la position du curseur
	rewind(_disks[id].disk_descriptor);

	return _NOERROR;
}

//_____________________________________________________________
// Fonctions externes
error start_disk(char *name, disk_id id){

	// Si l'id demandée n'est pas comprise entre 0 et MAX_OPEN_DISK-1
	if(!(id < MAX_OPEN_DISK))
		return _DISK_ID_TOO_BIG;
	// Si l'emplacement n'est pas libre pour cette id
	if(_disks[id].disk_descriptor != 0)
		return _DISK_ID_EXIST;

	// Ouverture en lecture écriture (Sans création du fichier)
	FILE* disk_file = fopen(name, "r+");
	// Si le disque (le fichier) n'existe pas
	if(disk_file == NULL)
		return _DISK_NOT_FOUND;

	// On déplace le curseur, on calcule le nombre de block
	fseek(disk_file, 0, SEEK_END);
	
	// On le démarre en lui affectant le descripteur de fichier et le flags _MOUNTED
	_disks[id].disk_descriptor = disk_file;
	_disks[id].nb_blocks = (ftell(disk_file) / BLCK_SIZE);
	_disks[id].flag = _MOUNTED;

	// On remet la position du curseur au début
	rewind(disk_file);

	return _NOERROR;
}

error stop_disk(disk_id id){
	// Si le disque est monté, on passe le flag en _UNMOUNTED puis on close le pointeur FILE
	if((_disks[id].flag & _MOUNTED) != 0){
		_disks[id].flag = _UNMOUNTED;
		fclose(_disks[id].disk_descriptor);	
		_disks[id].disk_descriptor = 0; // Remise du pointeur à zero
		return _NOERROR;
	}
	else
		return _DISK_UNMOUNTED;
}

error read_block(disk_id id, block b, uint32_t num){
	return read_physical_block(id, b, num);
}
error write_block(disk_id id, block b, uint32_t num){
	return write_physical_block(id, b, num);
}
error sync_disk(disk_id id){
	return _NOERROR;
}

// ##########################################################
// Fonctions informations DISQUE et PARTITIONS
// ##########################################################

error getFirstPartitionBlck(disk_id id, int partition, uint32_t *number){
	DISK_INFO infDisk;

	// Si le disque n'est pas monté
	if((_disks[id].flag & _MOUNTED) == 0)
		return _DISK_UNMOUNTED;

	// Récupération des informations du disque
	readDiskInfos(0, &infDisk);

	// Vérification de l'existence de la partition
	if(partition >= infDisk.nb_partitions)
		return _PARTITION_NOT_FOUND;

	// Calcul du premier block de la partition
	*number = 1;
	for(int i=0; i<partition; i++){
		*number += infDisk.p_sizes[i];
	}

	return _NOERROR;
}

error getPartitionSize(disk_id id, int partition, uint32_t *number){
	DISK_INFO infDisk;

	// Si le disque n'est pas monté
	if((_disks[id].flag & _MOUNTED) == 0)
		return _DISK_UNMOUNTED;

	// Récupération des informations du disque
	readDiskInfos(0, &infDisk);

	// Vérification de l'existence de la partition
	if(partition >= infDisk.nb_partitions)
		return _PARTITION_NOT_FOUND;

	*number = infDisk.p_sizes[partition];

	return _NOERROR;
}

error getFilesTableSize(disk_id id, int partition, uint32_t *number){
	PARTITION_INFO infPartition;

	// On récupère les informations de la partition
	error read_infos_part = readPartitionInfos(0, &infPartition, partition);
	if(read_infos_part != 0){
		return read_infos_part;
	}

	// Calcul de la taille nécessaire pour la table des fichiers (64o par entrée)
	// Dans un block de de la table on peut mettre 1024/64 = 16 fichiers
	if((infPartition.TTTFS_VOLUME_MAX_FILE_COUNT % 16) == 0)
		*number = (infPartition.TTTFS_VOLUME_MAX_FILE_COUNT / 16);
	else
		*number = (infPartition.TTTFS_VOLUME_MAX_FILE_COUNT / 16) + 1;

	return _NOERROR;
}

error readDiskInfos(disk_id id, DISK_INFO *infDisk){
	// Si le disque n'est pas monté
	if((_disks[id].flag & _MOUNTED) == 0)
		return _DISK_UNMOUNTED;

	// On lit le premier block du disque
	block b;
	read_block(id, b, 0);

	// Récupération du nombre de partitions
	infDisk->nb_partitions = readBlockToInt(b, 1);

	// Récupération des tailles des partitions
	//(+2 pour arriver aux entiers des tailles de partition présentes)
	for(int i=0; i< ((BLCK_SIZE/4)-2); i++){
		infDisk->p_sizes[i] = readBlockToInt(b, i+2);
	}

	return _NOERROR;
}

error readPartitionInfos(disk_id id, PARTITION_INFO *infPartition, int partition){
	uint32_t first_partition_blck = 0;
	block b;

	// Si le disque n'est pas monté
	if((_disks[id].flag & _MOUNTED) == 0)
		return _DISK_UNMOUNTED;

	error fst_blck = getFirstPartitionBlck(0, partition, &first_partition_blck);
	if(fst_blck != 0){
		return fst_blck;
	}

	// On lit le premier block de la partition
	read_block(id, b, first_partition_blck);

	// On lis les données du premier block de la partition (Description block)
	infPartition->TTTFS_MAGIC_NUMBER = readBlockToInt(b, 0);
	infPartition->TTTFS_VOLUME_BLOCK_SIZE = readBlockToInt(b, 1);
	infPartition->TTTFS_VOLUME_BLOCK_COUNT = readBlockToInt(b, 2);
	infPartition->TTTFS_VOLUME_FREE_BLOCK_COUNT = readBlockToInt(b, 3); 
	infPartition->TTTFS_VOLUME_FIRST_FREE_BLOCK = readBlockToInt(b, 4);
	infPartition->TTTFS_VOLUME_MAX_FILE_COUNT = readBlockToInt(b, 5); 
	infPartition->TTTFS_VOLUME_FREE_FILE_COUNT = readBlockToInt(b, 6);
	infPartition->TTTFS_VOLUME_FIRST_FREE_FILE = readBlockToInt(b, 7); 

	return _NOERROR;
}

error writePartitionInfos(disk_id id, PARTITION_INFO infPartition, int partition){
	uint32_t first_partition_blck = 0;
	block b;

	// Si le disque n'est pas monté
	if((_disks[id].flag & _MOUNTED) == 0)
		return _DISK_UNMOUNTED;

	error fst_blck = getFirstPartitionBlck(id, partition, &first_partition_blck);
	if(fst_blck != 0){
		return fst_blck;
	}

	// On lit le premier block de la partition
	read_block(id, b, first_partition_blck);

	// On écrit les données du premier block de la partition (Description block)
	writeIntToBlock(b, 0, infPartition.TTTFS_MAGIC_NUMBER);
	writeIntToBlock(b, 1, infPartition.TTTFS_VOLUME_BLOCK_SIZE);
	writeIntToBlock(b, 2, infPartition.TTTFS_VOLUME_BLOCK_COUNT);
	writeIntToBlock(b, 3, infPartition.TTTFS_VOLUME_FREE_BLOCK_COUNT);
	writeIntToBlock(b, 4, infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK);
	writeIntToBlock(b, 5, infPartition.TTTFS_VOLUME_MAX_FILE_COUNT); 
	writeIntToBlock(b, 6, infPartition.TTTFS_VOLUME_FREE_FILE_COUNT);
	writeIntToBlock(b, 7, infPartition.TTTFS_VOLUME_FIRST_FREE_FILE);

	error write_desc_block = write_block(id, b, first_partition_blck);
	if(write_desc_block != 0){
		stop_disk(0);
	}

	return _NOERROR;
}

void printInfoPartition(PARTITION_INFO infPartition){
	printf("%s\n", "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
	printf("MAGIC NUMBER -> %d\n", infPartition.TTTFS_MAGIC_NUMBER);
	printf("BLOCK_SIZE -> %d\n", infPartition.TTTFS_VOLUME_BLOCK_SIZE);
	printf("BLOCK_COUNT -> %d\n", infPartition.TTTFS_VOLUME_BLOCK_COUNT);
	printf("FREE_BLOCK_COUNT -> %d\n", infPartition.TTTFS_VOLUME_FREE_BLOCK_COUNT);
	printf("FIRST_FREE_BLOCK -> %d\n", infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK);
	printf("MAX_FILE_COUNT -> %d\n", infPartition.TTTFS_VOLUME_MAX_FILE_COUNT);
	printf("FREE_FILE_COUNT -> %d\n", infPartition.TTTFS_VOLUME_FREE_FILE_COUNT);
	printf("FIRST_FREE_FILE -> %d\n", infPartition.TTTFS_VOLUME_FIRST_FREE_FILE);
	printf("%s\n", "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
}

// ##########################################################
// Fonctions auxiliaires
// ##########################################################

char* strError(error err){
	if(err == _DISK_NOT_FOUND)
		return "Le disque n'existe pas.";
	if(err == _DISK_UNMOUNTED)
		return "Le disque n'est pas monté.";
	if(err == _NUM_BLCK_TOO_BIG)
		return "Le numéro de block est plus grand que le nombre total de blocks.";
	if(err == _DISK_ID_TOO_BIG)
		return "L'id demandée doit être inférieur à MAX_OPEN_DISK.";
	if(err == _DISK_ID_EXIST)
		return "L'id demandée existe déjà.";
	if(err == _READ_ERROR)
		return "La lecture du block à échoué.";
	if(err == _WRITE_ERROR)
		return "L'écriture du block à échoué.";
	if(err == _POS_IN_BLCK_TOO_BIG)
		return "La position est supérieur à BLCK_SIZE / 4 (256).";
	if(err == _PARTITION_NOT_FOUND)
		return "La partition n'existe pas sur le disque.";
	if(err == _MAX_FILES_TOO_BIG)
		return "Pas assez d'espace sur la partition.";
	if(err == _POS_IN_TABLE_TOO_BIG)
		return "Pas assez de place dans la table des fichiers.";
	if(err == _POS_IN_PARTITION_TOO_BIG)
		return "Pas assez de place dans la partition.";

	return "Aucune erreur.";
}

error eraseDisk(disk_id id, int block_debut, int block_fin){
	block b;
	for(int i=block_debut; i<block_fin; i++){
		// Lecture du block i
		error read = read_block(id, b, i);
		if(read != 0)
			return read;

		// Efface le block i
		error erase_blck = eraseBlock(b, 0, 256); 
		if(erase_blck != 0)
			return erase_blck;

		// Ecrit le block i
		error write = write_block(id, b, i);
		if(write != 0)
			return write;
	}
	return _NOERROR;
}

// ##########################################################
// Fonctions blocks
// ##########################################################

void printBlock(block b){
	int nb_block_print = 0;
	for(int i=0; i<BLCK_SIZE; i+=4){

		printf("%02hhX%02hhX%02hhX%02hhX ", (unsigned char)b[i], (unsigned char)b[i+1], (unsigned char)b[i+2], (unsigned char)b[i+3]);
		nb_block_print++;

		if(nb_block_print%8 == 0)
			printf("\n");
	}
}

void blockToLtleIndian(block b){
	unsigned char octet_0, octet_1, octet_2, octet_3;
	// Inversion par paquet de 4 octects
	for(int i=0; i < BLCK_SIZE; i+=4){
		octet_0 = b[i];
		octet_1 = b[i+1];
		octet_2 = b[i+2];
		octet_3 = b[i+3];

		b[i] = octet_3;
		b[i+1] = octet_2;
		b[i+2] = octet_1;
		b[i+3] = octet_0;
	}
}

error writeIntToBlock(block b, int position, uint32_t number){
	// Si la position est < (BLCK_SIZE/4) 
	if(position < (BLCK_SIZE/4)){
		position = (position * 4);

		b[position] = (number >> 24);
		b[position+1] = (number >> 16);
		b[position+2] = (number >> 8);
		b[position+3] = (number);

		return _NOERROR;
	}
	else
		return _POS_IN_BLCK_TOO_BIG;
}

error writeStrToBlock(block b, int position, char *str, int size){
	if((position + size) < BLCK_SIZE){

		int k = 0;
		while(str[k] != '\0'){
			b[position+k] = str[k];
			k++;
		}

		return _NOERROR;
	}
	else
		return _POS_IN_BLCK_TOO_BIG;
}

error readBlockToStr(block b, int position, char *str, int size){
	if((position + size) < BLCK_SIZE){

		for(int i=0; i<size; i++){
			str[i] = b[position+i];
		}

		return _NOERROR;
	}
	else
		return _POS_IN_BLCK_TOO_BIG;
}

error writeDirEntryToBlock(block b, int position, DIR_ENTRY dir_ent){
	// Si la position est < (BLCK_SIZE/32) 
	if(position < (BLCK_SIZE/32)){

		position = (position * 32);

		writeIntToBlock(b, position, dir_ent.number);
		writeStrToBlock(b, (position*4)+4, dir_ent.name, 28);

		return _NOERROR;
	}
	else
		return _POS_IN_BLCK_TOO_BIG;
}

error readBlockToDirEntry(block b, int position, DIR_ENTRY *dir_ent){
	// Si la position est < (BLCK_SIZE/32) 
	if(position < (BLCK_SIZE/32)){

		position = (position * 32);

		dir_ent->number = readBlockToInt(b, position);
		readBlockToStr(b, (position*4)+4, dir_ent->name, 28);

		return _NOERROR;
	}
	else
		return _POS_IN_BLCK_TOO_BIG;
}

uint32_t readBlockToInt(block b, int position){
	// Si la position est < (BLCK_SIZE/4)
	if(position < (BLCK_SIZE/4)){
		position = (position * 4);

		uint32_t number = 0;
		number |= (b[position] << 24);
		number |= (b[position+1] << 16);
		number |= (b[position+2] << 8);
		number |= (b[position+3]);

		return number;
	}
	else
		return -1;
}

error eraseBlock(block b, int debut, int fin){
	if( (debut < (BLCK_SIZE/4)) && (fin <= (BLCK_SIZE/4)) && (debut <= fin) ){

		for(int i=debut; i < fin; i++)
			writeIntToBlock(b, i, 0);

		return _NOERROR;
	}
	else
		return _POS_IN_BLCK_TOO_BIG;
}


// ##########################################################
// Fonctions table des fichiers
// ##########################################################

void initFilesTable(disk_id id, int partition){
	uint32_t size_table;
	// Création d'une entrée vierge
	FILE_ENTRY file; 
	initFileEntry(&file);

	getFilesTableSize(id, partition, &size_table);

	// Ecriture d'une entrée vierge dans tout les emplacements de la table
	for(int i=0; i < 16 * size_table; i++){
		// Initialisation de la chaine des entrées libres
		if(i < (16 * size_table)-1)
			setNextFreeFile(&file, i+1);
		else
			setNextFreeFile(&file, i);
		// Ecriture de l'entrée
		writeFileEntryToTable(id, partition, file, i);
	}
}

error writeFileEntryToTable(disk_id id, int partition, FILE_ENTRY file_ent, int file_pos){
	block b;
	uint32_t size_table;
	uint32_t first_partition_blck;

	// On récupère la taille de la table et le premier bloc de la partition
	getFilesTableSize(id, partition, &size_table);
	getFirstPartitionBlck(id, partition, &first_partition_blck);

	if(file_pos >= 16 * size_table)
		return _POS_IN_TABLE_TOO_BIG;


	int indx_blck_tab = file_pos / 16; // Position dans le bon bloc de la table
	int pos_in_blck = (file_pos % 16)*16; // Position du fichier dans le bloc

	// Lecture du bloc où l'on va écrire l'entrée de fichier
	read_block(id, b, (first_partition_blck + 1 + indx_blck_tab));

	// Ecriture dans le bloc de l'entrée de fichier
	writeIntToBlock(b, pos_in_blck, file_ent.tfs_size);
	writeIntToBlock(b, (pos_in_blck + 1), file_ent.tfs_type);
	writeIntToBlock(b, (pos_in_blck + 2), file_ent.tfs_subtype);
	for(int i=0; i<10; i++){
		writeIntToBlock(b, (pos_in_blck + 3 + i), file_ent.tfs_direct[i]);
	}
	writeIntToBlock(b, (pos_in_blck + 13), file_ent.tfs_indirect1);
	writeIntToBlock(b, (pos_in_blck + 14), file_ent.tfs_indirect2);
	writeIntToBlock(b, (pos_in_blck + 15), file_ent.tfs_next_free);

	// Réécriture du bloc
	write_block(id, b, (first_partition_blck + 1 + indx_blck_tab));

	return _NOERROR;
}

error readFileEntryFromTable(disk_id id, int partition, FILE_ENTRY *file_ent, int file_pos){
	block b;
	uint32_t size_table;
	uint32_t first_partition_blck;

	// On récupère la taille de la table et le premier bloc de la partition
	getFilesTableSize(id, partition, &size_table);
	getFirstPartitionBlck(id, partition, &first_partition_blck);

	if(file_pos >= 16 * size_table)
		return _POS_IN_TABLE_TOO_BIG;


	int indx_blck_tab = file_pos / 16; // Position dans le bon bloc de la table
	int pos_in_blck = (file_pos % 16)*16; // Position du fichier dans le bloc

	// Lecture du bloc qui contient l'entrée du fichier voulu
	read_block(id, b, (first_partition_blck + 1 + indx_blck_tab));

	// Ecriture de *file_ent
	file_ent->tfs_size = readBlockToInt(b, pos_in_blck);
	file_ent->tfs_type = readBlockToInt(b, (pos_in_blck + 1));
	file_ent->tfs_subtype = readBlockToInt(b, (pos_in_blck + 2));
	for(int i=0; i<10; i++){
		file_ent->tfs_direct[i] = readBlockToInt(b, (pos_in_blck + 3 + i));
	}
	file_ent->tfs_indirect1 = readBlockToInt(b, (pos_in_blck + 13));
	file_ent->tfs_indirect2 = readBlockToInt(b, (pos_in_blck + 14));
	file_ent->tfs_next_free = readBlockToInt(b, (pos_in_blck + 15));

	return _NOERROR;
}

void printFileEntry(FILE_ENTRY file_ent){
	printf("%s\n", "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
	printf("Size -> %d\n", file_ent.tfs_size);
	printf("Type -> %d\n", file_ent.tfs_type);
	printf("SubType -> %d\n", file_ent.tfs_subtype);
	printf("Direct_blck -> [");
	for(int i=0; i<10; i++){
		if(i<9)
			printf("%d, ", file_ent.tfs_direct[i]);
		else
			printf("%d]\n", file_ent.tfs_direct[i]);
	}
	printf("Indirect1 -> %d\n", file_ent.tfs_indirect1);
	printf("Indirect2 -> %d\n", file_ent.tfs_indirect2);
	printf("Next_free -> %d\n", file_ent.tfs_next_free);
	printf("%s\n", "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
}

// ##########################################################
// Fonctions entrées de fichier
// ##########################################################

void initFileEntry(FILE_ENTRY *file_ent){

	file_ent->tfs_size = 1024;
	file_ent->tfs_type = TFS_REGULAR;
	file_ent->tfs_subtype = 0;
	file_ent->tfs_indirect1 = 0;
	file_ent->tfs_indirect2 = 0;
	file_ent->tfs_next_free = 0;

	for(int i=0; i<10; i++){
		file_ent->tfs_direct[i] = 0;
	}
}

void setNextFreeFile(FILE_ENTRY *file_ent, uint32_t next){
	file_ent->tfs_next_free = next;
}

void setTypeFile(FILE_ENTRY *file_ent, int type){
	file_ent->tfs_type = type;
}

void setSubTypeFile(FILE_ENTRY *file_ent, int subtype){
	file_ent->tfs_subtype = subtype;
}

void addDirectBlock(FILE_ENTRY *file_ent, uint32_t num){
	// Ecriture sur la premiere case vide trouvée
	for(int i=0; i<10; i++){
		if(file_ent->tfs_direct[i] == 0){
			file_ent->tfs_direct[i] = num;
			break;
		}
	}
}




