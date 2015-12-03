#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../header/tfs_partition.h"
#include "../header/ll.h"

int main(int argc, char *argv[]){
	uint32_t nb_partitions = 0;
	// Test la présence du bon nombre d'arguments
	if(!((argc >= 4) && (argc % 2 == 0))){
		printf("%s\n", "Usage: tfs_partition -p size [-p size]... [name]");
		exit(1);
	}
	else{
		nb_partitions = (argc-2)/2;
		int partitions[nb_partitions];
		int total_partitions_size = 0;
		int pos_tab = 0;

		// Récupération des tailles
		for(int i=1; i<(argc-1); i+=2){
	 		if(strcmp(argv[i], "-p") == 0){
	 			partitions[pos_tab] = atoi(argv[i+1]);
	 			total_partitions_size += partitions[pos_tab];
	 		}
	 		else
	 		{
	 			printf("%s\n", "Usage: tfs_partition -p size [-p size]... [name]");
				exit(1);
			}
			pos_tab++;
	 	}

		// Vérification de l'existance du disque
		if(access(argv[argc-1], F_OK) == 0){

			uint32_t number_of_blocks = 0;
			block b;
			// On monte le disque en lui donnant l'id 0
			error start = start_disk(argv[argc-1], 0); 
			// Si le disque est bien démarré 
			if(start == 0){
				// On tente de lire le block 0
				error read = read_block(0, b, 0);
				// Si la lecture à réussie
				if(read == 0){
					// Lecture du nombre de blocks
					number_of_blocks = readBlockToInt(b, 0);

					// Vérification de la place disponible
					if(number_of_blocks <= total_partitions_size){
						fprintf(stderr, "La taille du disque n'est pas suffisante : %d blocks\n", (number_of_blocks-1));
						exit(1);
					}
					// Vérification de la présence de partionnement
					if(readBlockToInt(b,1) != 0){

						char validation[1024];

						printf("%s", "Le disque est déjà partitionné, voulez-vous l'écraser ? (y/n) ");
						scanf("%s", validation);
						if( (strcmp(validation, "Y") == 0) || (strcmp(validation, "y") == 0) )
							//eraseBlock(block b, int debut, int fin);
							printf("%s\n", "On efface !");
						else
							exit(1);

					}
					
					// Modification du block b
					// On insère le nombre en position 1 (sur 1024)
					writeIntToBlock(b, 1, nb_partitions);
					// On insère les tailles à la suite  
					for(int i=0; i<nb_partitions; i++){
						writeIntToBlock(b, i+2, partitions[i]); 
					}

					//printBlock(b);
					// Ecriture sur le block0 du block modifié
					error write = write_block(0, b, 0);
					if(write != 0)
						fprintf(stderr, "Erreur %d: %s\n", write, strError(write));

				}
				else{
					fprintf(stderr, "Erreur %d: %s\n", read, strError(read));
				}

			}
			else{
				fprintf(stderr, "Erreur %d: %s\n", start, strError(start));
			}

			// On démonte le disque
			error stop = stop_disk(0);
			if(stop != 0)
				fprintf(stderr, "Erreur %d: %s\n", stop, strError(stop));

		}
		else{
			fprintf(stderr, "%s : %s \n", "Erreur", "Le disque n'existe pas.");
			return 1;
		}

	}
	return 0;
}
