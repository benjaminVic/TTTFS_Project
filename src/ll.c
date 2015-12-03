#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../header/ll.h"

// Flags
#define _UNMOUNTED 0
#define _MOUNTED 1

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

	// Si le disque est monté
	if((_disks[id].flag & _MOUNTED) != 0){
		// Si le block voulu ne dépasse pas le nombre de blocks du disque
		if(num < _disks[id].nb_blocks){
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
		else
			return _NUM_BLCK_TOO_BIG;
	}
	else
		return _DISK_UNMOUNTED;
}

error write_physical_block(disk_id id, block b, uint32_t num){

	// Si le disque est monté
	if((_disks[id].flag & _MOUNTED) != 0){
		// Si le block voulu ne dépasse pas le nombre de blocks du disque
		if(num < _disks[id].nb_blocks){
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
		else
			return _NUM_BLCK_TOO_BIG;
	}
	else
		return _DISK_UNMOUNTED;
}

//_____________________________________________________________
// Fonctions externes
error start_disk(char *name, disk_id id){

	// Si l'id demandée est comprise entre 0 et MAX_OPEN_DISK-1
	if(id < MAX_OPEN_DISK){
		// Si l'emplacement est libre pour cette id (Le disque n'est pas monté)
		if(_disks[id].disk_descriptor == 0){

			// Ouverture en lecture écriture (Sans création du fichier)
			FILE* disk_file = fopen(name, "r+");
			// Si le disque (le fichier) existe
			if(disk_file != NULL){
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
			else
				return _DISK_NOT_FOUND;
		}
		else
			return _DISK_ID_EXIST;
	}
	else
		return _DISK_ID_TOO_BIG;
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

//_____________________________________________________________
// Fonctions auxiliaires
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
		return "La position est supérieur à BLCK_SIZE(1024).";

	return "Aucune erreur.";
}

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
	// Si la position est < BLCK_SIZE
	if(position < BLCK_SIZE){
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

int readBlockToInt(block b, int position){
	// Si la position est < BLCK_SIZE
	if(position < BLCK_SIZE){
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