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
	readDiskInfos(id, &infDisk);

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
	readDiskInfos(id, &infDisk);

	// Vérification de l'existence de la partition
	if(partition >= infDisk.nb_partitions)
		return _PARTITION_NOT_FOUND;

	*number = infDisk.p_sizes[partition];

	return _NOERROR;
}

error getFilesTableSize(disk_id id, int partition, uint32_t *number){
	PARTITION_INFO infPartition;

	// On récupère les informations de la partition
	error read_infos_part = readPartitionInfos(id, &infPartition, partition);
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

	error fst_blck = getFirstPartitionBlck(id, partition, &first_partition_blck);
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
	if(err == _TABLE_IS_FULL)
		return "La table des fichiers est pleine.";
	if(err == _CANNOT_REMOVE_FILE_ENTRY)
		return "La racine ne peut-être supprimée.";
	if(err == _PARTITION_IS_FULL)
		return "Plus aucun bloc n'est libre.";
	if(err == _DIRECT_TAB_IS_FULL)
		return "Il n'y à plus de place le tableau de blocs direct.";
	if(err == _DIRECT_TAB_IS_EMPTY)
		return "Le tableau de blocs direct est vide.";

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

error initFilesTable(disk_id id, int partition){
	uint32_t size_table;
	// Création d'une entrée vierge
	FILE_ENTRY file; 
	initFileEntry(&file);

	error getSize = getFilesTableSize(id, partition, &size_table);
	if(getSize != 0)
		return getSize;

	// Ecriture d'une entrée vierge dans tout les emplacements de la table
	for(int i=0; i < 16 * size_table; i++){
		// Initialisation de la chaine des entrées libres
		if(i < (16 * size_table)-1)
			file.tfs_next_free = i+1;
		else
			file.tfs_next_free = i;
		// Ecriture de l'entrée
		writeFileEntryToTable(id, partition, file, i);
	}

	return _NOERROR;
}

error writeFileEntryToTable(disk_id id, int partition, FILE_ENTRY file_ent, int file_pos){
	block b;
	uint32_t size_table;
	uint32_t first_partition_blck;

	// On récupère la taille de la table et le premier bloc de la partition
	error getSize = getFilesTableSize(id, partition, &size_table);
	if(getSize != 0)
		return getSize;

	error getFirst = getFirstPartitionBlck(id, partition, &first_partition_blck);
	if(getFirst != 0)
		return getFirst;

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
	error getSize = getFilesTableSize(id, partition, &size_table);
	if(getSize != 0)
		return getSize;

	error getFirst = getFirstPartitionBlck(id, partition, &first_partition_blck);
	if(getFirst != 0)
		return getFirst;

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

error addFileEntryToTable(disk_id id, int partition, int file_size, int file_type, int file_subtype){
	PARTITION_INFO infPartition;
	FILE_ENTRY newFileEntry;
	FILE_ENTRY oldFileEntry;

	// Récupération des informations
	error read_infos_part = readPartitionInfos(id, &infPartition, partition);
	if(read_infos_part != 0)
		return read_infos_part;

	// Si il n'y a plus de place dans la table des fichiers
	if(infPartition.TTTFS_VOLUME_FREE_FILE_COUNT == 0)
		return _TABLE_IS_FULL;

	// Lecture de la premiere entrée libre
	error read_file_entry = readFileEntryFromTable(id, partition, &oldFileEntry, infPartition.TTTFS_VOLUME_FIRST_FREE_FILE);
	if(read_file_entry != 0)
		return read_file_entry;


	// Renseignement des informations avant écriture sur le disque
	initFileEntry(&newFileEntry);
	newFileEntry.tfs_type = file_type;
	newFileEntry.tfs_subtype = file_subtype;
	newFileEntry.tfs_size = file_size;
	newFileEntry.tfs_next_free = 0;

	// Ecriture de la racine dans le premier emplacement libre de la table
	error write_file_entry = writeFileEntryToTable(id, partition, newFileEntry, infPartition.TTTFS_VOLUME_FIRST_FREE_FILE);
	if(write_file_entry != 0)
		return write_file_entry;

	// Modification des informations
	infPartition.TTTFS_VOLUME_FREE_FILE_COUNT -= 1;
	infPartition.TTTFS_VOLUME_FIRST_FREE_FILE = oldFileEntry.tfs_next_free;

	// Ecriture des nouvelles informations
	error write_infos_part = writePartitionInfos(id, infPartition, partition);
	if(write_infos_part != 0)
		return write_infos_part;

	return _NOERROR;
}

error removeFileEntryInTable(disk_id id, int partition, int file_pos){
	uint32_t size_table;
	PARTITION_INFO infPartition;
	FILE_ENTRY emptyFile;

	if(file_pos == 0)
		return _CANNOT_REMOVE_FILE_ENTRY;

	// On récupère la taille de la table
	error getSize = getFilesTableSize(id, partition, &size_table);
	if(getSize != 0)
		return getSize;

	if(file_pos >= 16 * size_table)
		return _POS_IN_TABLE_TOO_BIG;

	// Récupération des informations
	error read_infos_part = readPartitionInfos(id, &infPartition, partition);
	if(read_infos_part != 0)
		return read_infos_part;

	// On initialise une nouvelle entrée en la chainant avec la premiere actuellement libre
	initFileEntry(&emptyFile);
	emptyFile.tfs_next_free = infPartition.TTTFS_VOLUME_FIRST_FREE_FILE;

	// Ecriture de l'entrée libre
	error write_file_entry = writeFileEntryToTable(id, partition, emptyFile, file_pos);
	if(write_file_entry != 0)
		return write_file_entry;

	// On donne au FIRST_FREE_FILE la valeur de la position du fichier effacé
	infPartition.TTTFS_VOLUME_FIRST_FREE_FILE = file_pos;
	// Puis on incrémente le nombre de fichier disponibles
	infPartition.TTTFS_VOLUME_FREE_FILE_COUNT += 1;

	// Ecriture des nouvelles informations
	error write_infos_part = writePartitionInfos(id, infPartition, partition);
	if(write_infos_part != 0)
		return write_infos_part;

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
// Fonctions table des fichiers
// ##########################################################

error initFreeBlockChain(disk_id id, int partition){
	uint32_t size_table;
	uint32_t first_partition_blck;
	PARTITION_INFO infPartition;
	block b;

	error getSize = getFilesTableSize(id, partition, &size_table);
	if(getSize != 0)
		return getSize;

	error fst_blck = getFirstPartitionBlck(id, partition, &first_partition_blck);
	if(fst_blck != 0){
		return fst_blck;
	}

	// Récupération des informations
	error read_infos_part = readPartitionInfos(id, &infPartition, partition);
	if(read_infos_part != 0)
		return read_infos_part;

	eraseBlock(b, 0, 256); // Mise à zero du bloc

	// Chainage des blocs
	int k = infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK;
	for(int i=(first_partition_blck + k); i <= infPartition.TTTFS_VOLUME_BLOCK_COUNT; i++){
		
		if(i < infPartition.TTTFS_VOLUME_BLOCK_COUNT)
			writeIntToBlock(b, 255, k+1);
		else
			writeIntToBlock(b, 255, k);

		error write = write_block(id, b, i);
		if(write != 0)
			return write;

		k++;
	}

	return _NOERROR;
}

error addBlockInChain(disk_id id, int partition, int number){
	uint32_t first_partition_blck;
	uint32_t size_table;
	PARTITION_INFO infPartition;
	block b;

	// Le premier bloc de la partition
	error fst_blck = getFirstPartitionBlck(id, partition, &first_partition_blck);
	if(fst_blck != 0){
		return fst_blck;
	}

	// On récupère la taille de la table
	error getSize = getFilesTableSize(id, partition, &size_table);
	if(getSize != 0)
		return getSize;

	// Récupération des informations
	readPartitionInfos(id, &infPartition, partition);

	if((number < size_table) || (number >= infPartition.TTTFS_VOLUME_BLOCK_COUNT))
		return _NUM_BLCK_TOO_BIG;

	// Lecture du bloc pour le chainer avec le premier bloc de la chaine
	int k = number + first_partition_blck;
	read_block(id, b, k);
	writeIntToBlock(b, 255, infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK);

	infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK = number;
	
	infPartition.TTTFS_VOLUME_FREE_BLOCK_COUNT += 1;

	// Réecriture du bloc et des informations de partition
	writePartitionInfos(id, infPartition, partition);
	write_block(id, b, k);	

	return _NOERROR;
}

error removeFirstBlockInChain(disk_id id, int partition){
	uint32_t first_partition_blck;
	PARTITION_INFO infPartition;
	block b;

	// Le premier bloc de la partition
	error fst_blck = getFirstPartitionBlck(id, partition, &first_partition_blck);
	if(fst_blck != 0){
		return fst_blck;
	}

	// Récupération des informations
	readPartitionInfos(id, &infPartition, partition);

	// Lecture du premier bloc de la chaine pour récupéré et effacer le dernier nombre (le prochain bloc libre)
	int k = infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK + first_partition_blck;
	read_block(id, b, k);
	
	infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK = readBlockToInt(b, 255);
	writeIntToBlock(b, 255, 0);

	infPartition.TTTFS_VOLUME_FREE_BLOCK_COUNT -= 1;

	// Réecriture du bloc et des informations de partition
	writePartitionInfos(id, infPartition, partition);
	write_block(id, b, k);

	return _NOERROR;
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

error addNewBlock(disk_id id, int partition, int file_pos){
	uint32_t size_table;
	PARTITION_INFO infPartition;
	FILE_ENTRY file;

	// On récupère la taille de la table
	error getSize = getFilesTableSize(id, partition, &size_table);
	if(getSize != 0)
		return getSize;

	if(file_pos >= 16 * size_table)
		return _POS_IN_TABLE_TOO_BIG;

	// Récupération des informations
	readPartitionInfos(id, &infPartition, partition);

	if(infPartition.TTTFS_VOLUME_FREE_BLOCK_COUNT == 0)
		return _PARTITION_IS_FULL;

	// On récupère l'entrée en question
	readFileEntryFromTable(id, partition, &file, file_pos);

	// On ajoute le premier bloc libre de la partition dans le premier emplacement libre (!0) des blocs direct
	if(file.tfs_direct[9] != 0)
		return _DIRECT_TAB_IS_FULL;

	for(int i=0; i<10; i++){
		if(file.tfs_direct[i] == 0){
			file.tfs_direct[i] = infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK;
			break;
		}
	}

	// Ecriture de l'entrée modifiée
	writeFileEntryToTable(id, partition, file, file_pos);

	// Suppression du premier bloc du chainage
	removeFirstBlockInChain(id, partition);

	return _NOERROR;
}

error removeLastBlock(disk_id id, int partition, int file_pos){
	uint32_t size_table;
	FILE_ENTRY file;

	// On récupère la taille de la table
	error getSize = getFilesTableSize(id, partition, &size_table);
	if(getSize != 0)
		return getSize;

	if(file_pos >= 16 * size_table)
		return _POS_IN_TABLE_TOO_BIG;

	// On récupère l'entrée en question
	readFileEntryFromTable(id, partition, &file, file_pos);

	// On retire le dernier bloc direct
	if(file.tfs_direct[0] == 0)
		return _DIRECT_TAB_IS_EMPTY;

	for(int i=1; i<10; i++){
		if(file.tfs_direct[i] == 0){
			addBlockInChain(id, partition, file.tfs_direct[i-1]);
			file.tfs_direct[i-1] = 0;
			break;
		}
	}

	// Ecriture de l'entrée modifiée
	writeFileEntryToTable(id, partition, file, file_pos);


	return _NOERROR;
}


