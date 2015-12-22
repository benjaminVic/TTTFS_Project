#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../header/tfs_create.h"
#include <stdint.h>

int main(int argc, char *argv[]){
	uint32_t size = 0;

	// Test la présence du bon nombre d'arguments
	if((argc < 3) || (argc > 4)){
		printf("%s\n", "Usage: tfs_create -s size [name]");
		exit(1);
	}
	// Récupération de la taille
	if(strcmp(argv[1],"-s") == 0){
		size = atoi(argv[2]);
	}
	else{
		printf("%s : %s\n", "Invalide option", argv[1]);
		exit(1);
	}
	// Création avec le nom par défault "disk.tfs"
	if(argc == 3){
		if(create_disk(size, NULL) == -1)
			exit(1);
	}
	// Création avec le nom donné en argument
	else if(argc == 4){
		if(create_disk(size, argv[3]) == -1)
			exit(1);
	}
	return 0;
}


int create_disk(uint32_t size, char* name){
	char disk_name[1024];
	FILE* file;

	if(name == NULL)
		strncpy(disk_name, "disk.tfs", 1024);
	else{
		strncpy(disk_name, name, 1024);
		strcat(disk_name, ".tfs");
	}

	// Vérification de l'existance du disque
	if(access(disk_name, F_OK) == -1){

		unsigned char* buffer = (unsigned char*) malloc(BLCK_SIZE * sizeof(char)); // Buffer pour stocker un block de 1024 octect
		unsigned char bytes[4]; // Les 4 premiers octects pour la taille du disque

		// Ouverture du fichier (Création)
		if((file = fopen(disk_name, "w")) == NULL){
			fprintf(stderr, "%s : \n", "Erreur lors de la création du disque");
			perror("");
			return -1;
		}

		// Remplissage du disque avec des block vides
		fwrite(buffer, 1024, size, file);

		// Masquage pour séparé les 4 octets de la taille du disque 	
		bytes[0] = (size >> 24) & 0xFF;
		bytes[1] = (size >> 16) & 0xFF;
		bytes[2] = (size >> 8) & 0xFF;
		bytes[3] = size & 0xFF;

		// Stockage de la taille en little_indian sur les 4 premiers octects du block 1
		rewind(file);
		fprintf(file, "%c%c%c%c", (unsigned char)bytes[3], (unsigned char)bytes[2], (unsigned char)bytes[1], (unsigned char)bytes[0]);

		fclose(file);
		free(buffer);
	}
	else{
		fprintf(stderr, "%s : %s \n", "Erreur", "Le disque existe déjà.");
		return -1;
	}

	return 0;
}
