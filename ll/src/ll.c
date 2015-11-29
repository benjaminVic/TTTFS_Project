#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../header/ll.h"

DISK _disks[MAX_OPEN_DISK] = {};


int main(int argc, char *argv[]){
	//****************************
	// ZONE DE TEST
	//****************************

	disk_id disk0;
	disk0.id = 0;

	start_disk("zDisk.tfs", disk0);


	return 0;
}

error read_physical_block(disk_id id, block b, uint32_t num){
	error errorTest;
	errorTest.errno = 0;
	return errorTest;
}

error write_physical_block(disk_id id, block b, uint32_t num){
	error errorTest;
	errorTest.errno = 0;
	return errorTest;
}

error start_disk(char *name, disk_id *id){
	DISK* disk = 0;
	int disk_file = 0;


	error errorTest = {0};
	return errorTest;	
}