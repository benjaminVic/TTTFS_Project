#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../header/tfs_format.h"
#include "../header/tfs.h"

int main(int argc, char *argv[]){
	// Test la présence du bon nombre d'arguments
	if(argc != 6){
		printf("%s\n", "Usage: tfs_format -p partition -mf file_count [disk]");
		exit(1);
	}
	else{
		int num_partition;
		int file_count;
		char* disk_name;
		uint32_t nb_partitions = 0;

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
	}
	return 0;
}
