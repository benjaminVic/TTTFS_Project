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
error read_physical_block(disk_id id, block b, uint32_t num);
error write_physical_block(disk_id id, block b, uint32_t num);

// Emplacements disponibles pour les disques
DISK _disks[MAX_OPEN_DISK] = {};

//_____________________________________________________________
// ZONE DE TEST (Les opérations ici ce feront dans la couche supérieure)
int main(int argc, char *argv[]){

	block block_lu;
	error start = start_disk("zDisk.tfs", 0);
	error stop = stop_disk(0);

	// Si le disque est bien démarré 
	if(start == 0){
		// On tente de lire le block x 
		error read = read_physical_block(0, block_lu, 0);
		// Si la lecture à réussie, on l'affiche
		if(read == 0){
			printBlock(block_lu);
		}
		else{
			fprintf(stderr, "Erreur %d: %s\n", read, strError(read));
		}
	}
	else{
		fprintf(stderr, "Erreur %d: %s\n", start, strError(start));
	}
	return 0;
}
//_____________________________________________________________

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

			return _NOERROR;	
		}
		else
			return _NUM_BLCK_TOO_BIG;
	}
	else
		return _DISK_UNMOUNTED;
}

error write_physical_block(disk_id id, block b, uint32_t num){
	return _NOERROR;
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
		return _NOERROR;
	}
	else
		return _DISK_UNMOUNTED;
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