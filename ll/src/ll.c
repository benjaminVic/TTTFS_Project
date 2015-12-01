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
	
	// Si le disque est bien démarré 
	if(start == 0){
		// On tente de lire le block x 
		error read = read_physical_block(0, block_lu, 0);
		// Si la lecture à réussie, on l'affiche
		if(read == 0){
			//printf("%s\n", block_lu);
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
		// On lit les 1024 octects 
		return _NOERROR;
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

			int disk_file = open(name, O_RDWR);
			// Si le disque (le fichier) existe
			if(disk_file != -1){
				// On le démarre en lui affectant le descripteur de fichier et le flags _MOUNTED
				_disks[id].disk_descriptor = disk_file;
				_disks[id].flag = _MOUNTED;

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
	return _NOERROR;
}

//_____________________________________________________________
// Fonctions auxiliaires
char* strError(error err){
	if(err == _DISK_NOT_FOUND)
		return "Le disque n'existe pas.";
	if(err == _DISK_UNMOUNTED)
		return "Le disque n'est pas monté.";
	if(err == _BLCK_NOT_FOUND)
		return "Le numéro de block est plus grand que la taille du disque.";
	if(err == _DISK_ID_TOO_BIG)
		return "L'id demandée doit être inférieur à MAX_OPEN_DISK.";
	if(err == _DISK_ID_EXIST)
		return "L'id demandée existe déjà.";

	return "Aucune erreur.";
}