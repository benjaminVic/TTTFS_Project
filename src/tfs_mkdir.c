#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../header/tfs_partition.h"
#include "../header/ll.h"

#define HOST_LOCATION 		0
#define PARTITION_NUMBER	1
#define TFS_LOCATION			2


/**
 * @param[in]  src path
 * @param[out] host_location
 * @param[out] partition 
 * @param[out] tfs_location 
 */
void parse_path(char* path, char* host_location, int* partition, char* tfs_location)
{

	char source[240];
	char* token;
	char* file_extension;
	uint8_t token_type= HOST_LOCATION; 

	strcpy(source, path);
	token = strtok(source, "/");

	while(token != NULL) 
  {
		switch(token_type)
		{
			case HOST_LOCATION:
				strcat(host_location, "/");
				strcat(host_location, token);
				
				file_extension = strstr(token, ".");
				if(file_extension != NULL)
				{
					if(strcmp (".tfs", file_extension) == 0)
					{
						token_type= PARTITION_NUMBER;
					}
				}
			break;

			case PARTITION_NUMBER:
				*partition= atoi(token);
				token_type= TFS_LOCATION;
			break;

			case TFS_LOCATION:
				strcat(tfs_location, "/");
				strcat(tfs_location, token);
			break;
		}
		token = strtok(NULL, "/");	
  }
}

void test_parse_path()
{
	int partition= -1;
	char path[]= "/home/jerome_skoda/Documents/Bullshif.tfs/15/bla/blo/bli/blio";
	char host_location[240]= "";
	char tfs_location[240]= "";

	parse_path(path, host_location, &partition, tfs_location);

	printf("test_parse_path()\npath:%s \nhost_location: %s \npartition: %d \ntfs_location: %s \n", 
		path, 
		host_location, 
		partition, 
		tfs_location);
}


int main(int argc, char *argv[])
{
	int partition=-1;
	char host_location[240]= "";
	char tfs_location[240]= "";

	// Test la présence du bon nombre d'arguments
	if(argc != 2){
		printf("%s\n", "Usage: tfs_mkdir [name]");
		exit(1);
	}

	parse_path(argv[1], host_location, &partition, tfs_location);

	// Vérification de l'existance du disque
	if(access(host_location, F_OK) != 0){
		fprintf(stderr, "%s : %s \n", "Erreur", "Le disque n'existe pas.");
		exit(1);
	}

	// On monte le disque en lui donnant l'id 0
	error start = start_disk(host_location, 0); 

	// Si le disque n'est pas monté
	if(start != 0){
		fprintf(stderr, "Erreur %d: %s\n", start, strError(start));
		exit(1);
	}

	// Lecture des information de la partition
	PARTITION_INFO infPartition;
	readPartitionInfos(0, &infPartition, partition);

	// Lecture de la racine
	FILE_ENTRY racine;
	readFileEntryFromTable(0, partition, &racine, 0);


 printFileEntry(racine);



	stop_disk(0);
	return _NOERROR;
}
