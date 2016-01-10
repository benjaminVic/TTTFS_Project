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
	uint32_t first_partition_blck = 0;
	PARTITION_INFO infPartition;
	DIR_ENTRY dir_ent;
	FILE_ENTRY file_ent;
	error err;
	int file_pos= 0;

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
	err = start_disk(host_location, 0); 
	if(err != 0){
		fprintf(stderr, "Erreur %d: %s\n", err, strError(err));
		exit(1);
	}

	// recup le premier block de la partition
	err = getFirstPartitionBlck(0, partition, &first_partition_blck);
	if(err != 0){
		fprintf(stderr, "Erreur %d: %s\n", err, strError(err));
		stop_disk(0);
		return err;
	}

	// Récupération des informations
	err = readPartitionInfos(0, &infPartition, partition);
	if(err != 0)
	{
		fprintf(stderr, "Erreur %d: %s\n", err, strError(err));
		stop_disk(0);
		return err;
	}



	printInfoPartition(infPartition);
	

	uint32_t new_block_n= infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK;
	uint32_t new_file_n = infPartition.TTTFS_VOLUME_FIRST_FREE_FILE;

	addFileEntryToTable(0, partition, 1, TFS_DIRECTORY, 0);
 	addNewBlock(0, partition, new_block_n);

	DIR_ENTRY new_entry;
	new_entry.number = new_file_n;
	sprintf(new_entry.name, "new_entry");

	FILE_ENTRY  parent_f;
	block 			parent_b;


	uint32_t 		parent_f_n=0;

	for(parent_f_n= 0; parent_f.name[0] != '\0'; parent_f_n++)
	{

		readFileEntryFromTable(0, partition, &parent_f, parent_f_n);

		if(strcmp ( parent_f.name,  "balbal") ==  0)
		{
				read_block(0, parent_b, (first_partition_blck + parent_f.tfs_direct[0]));
				writeDirEntryToBlock(b_racine, 2, new_entry);
				write_block(0, b_racine, (first_partition_blck + parent_f.tfs_direct[0]));
		}

	}

	DIR_ENTRY dot;
	DIR_ENTRY double_dot;

	dot.number = new_file_n;
	sprintf(dot.name, ".");

	double_dot.number = parent_file_n;
	sprintf(double_dot.name, "..");

	block 			new_block;
	read_block(0, new_block, new_block_n);
	writeDirEntryToBlock(new_block, 0, dot);
	writeDirEntryToBlock(new_block, 1, double_dot);
	write_block(0, new_block,  new_block_n);







	FILE_ENTRY tesst;
	readFileEntryFromTable(0, partition, &tesst, 0);




	block b_test;
	read_block(0,  b_test, new_block_n);


	for(file_pos=0; file_pos < 10; file_pos++)
	{

DIR_ENTRY testEntry;

			readBlockToDirEntry(b_test, file_pos, &testEntry);
			printf("%s\n", testEntry.name);

	
	}






	stop_disk(0);
	return _NOERROR;
}
