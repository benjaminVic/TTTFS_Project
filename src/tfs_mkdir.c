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


DIR_ENTRY search_dir(block b, char* name)
{
  uint32_t file_pos= 0;
	DIR_ENTRY dir_entry;

	do
	{
		readBlockToDirEntry(b, file_pos, &dir_entry);
		if(strcmp (dir_entry.name,  name) == 0)
		{
			return dir_entry;
		}
		file_pos++;
	} while (dir_entry.name[0] != '\0');

dir_entry.number = -1;

	return dir_entry;
}

error add_new_dir(uint32_t partition, char * nom, uint32_t parent_file_pos)
{	
	// Récupération des informations
	PARTITION_INFO infPartition;
	error err = readPartitionInfos(0, &infPartition, partition);
	if(err != 0)
	{
		fprintf(stderr, "Erreur %d: %s\n", err, strError(err));
		stop_disk(0);
		return err;
	}

	// recup le premier block de la partition
	uint32_t first_partition_blck = 0;
	err = getFirstPartitionBlck(0, partition, &first_partition_blck);
	if(err != 0){
		fprintf(stderr, "Erreur %d: %s\n", err, strError(err));
		stop_disk(0);
		return err;
	}

	uint32_t new_file_pos = infPartition.TTTFS_VOLUME_FIRST_FREE_FILE;
	addFileEntryToTable(0, partition, new_file_pos, TFS_DIRECTORY, 2);
	uint32_t new_block_pos= first_partition_blck + infPartition.TTTFS_VOLUME_FIRST_FREE_BLOCK;
 	addNewBlock(0, partition, new_block_pos);

	DIR_ENTRY new_entry;
	new_entry.number = new_file_pos;
	strcpy(new_entry.name, nom);

	FILE_ENTRY parent_file;
	readFileEntryFromTable(0, partition, &parent_file, parent_file_pos);
	FILE_ENTRY new_file;
	readFileEntryFromTable(0, partition, &new_file   , new_file_pos   );

	block       parent_block;
	read_block(0, parent_block, (first_partition_blck + parent_file.tfs_direct[0]));
	block  			new_block;
	read_block(0, new_block,    (first_partition_blck + new_file.tfs_direct[0]   ));

  // recherche du prochain emplacement libre
	DIR_ENTRY dir_entry;
  uint32_t file_pos= 0;
  do
	{
		readBlockToDirEntry(parent_block, file_pos, &dir_entry);
		file_pos++;
	} while (dir_entry.name[0] != '\0');
  file_pos--;

	err = writeDirEntryToBlock(parent_block, file_pos, new_entry);
	if(err != 0){
		fprintf(stderr, "Erreur %d: %s\n", err, strError(err));
		stop_disk(0);
		return err;
	}


	write_block(0, parent_block, (first_partition_blck + parent_file.tfs_direct[0]   ) );


	read_block(0, new_block, (first_partition_blck + new_file.tfs_direct[0]) );

	DIR_ENTRY dot;
	dot.number = new_file_pos;
	sprintf(dot.name, ".");
	writeDirEntryToBlock(new_block, 0, dot);

	DIR_ENTRY double_dot;
	double_dot.number = parent_file_pos;
	sprintf(double_dot.name, "..");
	writeDirEntryToBlock(new_block, 1, double_dot);

	write_block(0, new_block, (first_partition_blck + new_file.tfs_direct[0]) );
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

	// recup le premier block de la partition
	err = getFirstPartitionBlck(0, partition, &first_partition_blck);
	if(err != 0){
		fprintf(stderr, "Erreur %d: %s\n", err, strError(err));
		stop_disk(0);
		return err;
	}


	printInfoPartition(infPartition);
	


	FILE_ENTRY  parent_file;
	block 			parent_block;
	uint32_t 		parent_file_number=0;
	FILE_ENTRY  file_entry;





//

	DIR_ENTRY   parent_dir_entry;
	parent_dir_entry.number = 0; // racine
	char* token = strtok(tfs_location, "/");
	char dirname[240];

	while(token != NULL) 
  {
		strcpy(dirname, token);
		parent_file_number= parent_dir_entry.number;
    
		printf("token: %s, parent: %d number: %d\n", token, parent_file_number, parent_dir_entry.number);

		readFileEntryFromTable(0, partition, &parent_file, parent_file_number);
		read_block(0, parent_block, (first_partition_blck + parent_file.tfs_direct[0]));

		parent_dir_entry = search_dir(parent_block, token);



		printf("token: %s, parent: %d number: %d\n", token, parent_file_number, parent_dir_entry.number);


		if(parent_dir_entry.number == -1)
		{
			token = strtok(NULL, "/");	
		 	if(token == NULL)
			{
				add_new_dir(partition, dirname, parent_file_number);
			}
			else
			{
				fprintf(stderr, "Le dossier: %s n'existe pas.", dirname);
			}
			break;
		}
		token = strtok(NULL, "/");	
		
		if(token == NULL)
		{
			fprintf(stderr, "Le dossier: %s existe déjà.", dirname);
			break;
		}
	}


/*

add_new_dir(partition, parent_block_n, new_block_n);



	FILE_ENTRY tesst;
	readFileEntryFromTable(0, partition, &tesst,  new_file_n );



	block b_test;
	read_block(0,  b_test, new_block_n );

*/


	readFileEntryFromTable(0, partition, &parent_file, parent_dir_entry.number);
	read_block(0, parent_block, (first_partition_blck + parent_file.tfs_direct[0]));


	for(file_pos=0; file_pos < 8; file_pos++)
	{

DIR_ENTRY testEntry;

			readBlockToDirEntry(parent_block, file_pos, &testEntry);
			printf("%s\n", testEntry.name);

	
	}






	stop_disk(0);
	return _NOERROR;
}





